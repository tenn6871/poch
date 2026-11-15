#include <modtopia/modtopia.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>

MT_DECLARE_MOD()

/* Pokopia 2.0.1a (main build 76a489f3...), guest addresses from the
   published Modtopia mappings and matching 2.0.1a memory layouts. */
enum {
    WORLD_PERMISSIONS = 0x8c,
    ONLINE_FLAG = 0x90,
    COINS_OFFSET = 0x7416ac,
    PP_OFFSET = 0x250c58,
    BAG_OFFSET = 0x13d4920 + 0x830,
    BAG_SLOTS = 120,
    SLOT_SIZE = 16,
    SLOT_VTABLE_OFFSET = 0x322c1d8
};

static const MtHost *H;
static uint64_t g_save_ptr, g_game_mgr_ptr, g_slot_vtable;
static int g_f5_was_down;
static volatile LONG g_offline;
static volatile LONG g_water_seen, g_pp_peak;
static int8_t g_water_offsets[100][2];

static void build_water_offsets(void) {
    unsigned n = 0;
    for (int z = -4; z <= 5; ++z)
        for (int x = -4; x <= 5; ++x) {
            g_water_offsets[n][0] = (int8_t)x;
            g_water_offsets[n++][1] = (int8_t)z;
        }
    for (unsigned i = 1; i < 100; ++i) {
        int8_t x = g_water_offsets[i][0], z = g_water_offsets[i][1];
        int rx = x < 0 ? -x : x, rz = z < 0 ? -z : z;
        int ring = rx > rz ? rx : rz, distance = x * x + z * z;
        unsigned j = i;
        while (j > 0) {
            int px = g_water_offsets[j - 1][0], pz = g_water_offsets[j - 1][1];
            int prx = px < 0 ? -px : px, prz = pz < 0 ? -pz : pz;
            int pring = prx > prz ? prx : prz;
            if (pring < ring || (pring == ring && px * px + pz * pz <= distance)) break;
            g_water_offsets[j][0] = (int8_t)px;
            g_water_offsets[j][1] = (int8_t)pz;
            --j;
        }
        g_water_offsets[j][0] = x;
        g_water_offsets[j][1] = z;
    }
}

static int water_count(MtMod *mod) {
    const char *choice = H->config_string(mod, "water_area");
    if (!choice) return 0;
    if (strncmp(choice, "25", 2) == 0) return 25;
    if (strncmp(choice, "50", 2) == 0) return 50;
    if (strncmp(choice, "75", 2) == 0) return 75;
    if (strncmp(choice, "100", 3) == 0) return 100;
    return 0;
}

static int on_frame(MtMod *mod, const MtEvent *event, void *user) {
    (void)event;
    (void)user;
    const int f5_down = H->key_down(mod, "F5") != 0;
    const int add_coins = f5_down && !g_f5_was_down;
    g_f5_was_down = f5_down;
    InterlockedExchange(&g_offline, 0);
    uint64_t save = 0, mgr = 0;
    uint8_t session = 0;
    uint32_t permissions = 0;
    if (H->mem_read(g_game_mgr_ptr, &mgr, sizeof mgr) != MT_OK || !mgr ||
        H->mem_read(mgr + WORLD_PERMISSIONS, &permissions, sizeof permissions) != MT_OK || !permissions ||
        H->mem_read(mgr + ONLINE_FLAG, &session, sizeof session) != MT_OK || (session & 1))
        return MT_EVENT_CONTINUE;
    InterlockedExchange(&g_offline, 1);
    if (H->mem_read(g_save_ptr, &save, sizeof save) != MT_OK || !save)
        return MT_EVENT_CONTINUE;

    if (add_coins && H->config_bool(mod, "coins")) {
        int32_t coins = 0;
        if (H->mem_read(save + COINS_OFFSET, &coins, sizeof coins) == MT_OK &&
            coins >= 0 && coins <= INT32_MAX - 1000) {
            const int32_t added = coins + 1000;
            if (H->mem_write(save + COINS_OFFSET, &added, sizeof added) == MT_OK &&
                MT_HOST_HAS(H, notify) && H->notify)
                H->notify(mod, "라이프 코인 +1,000", 2.0);
        }
    }
    if (H->config_bool(mod, "pp")) {
        uint16_t pp[2] = {0, 0};
        if (H->mem_read(save + PP_OFFSET, pp, sizeof pp) == MT_OK &&
            pp[1] > 0 && pp[1] <= 10000 && pp[0] <= 10000) {
            LONG peak = InterlockedCompareExchange(&g_pp_peak, 0, 0);
            LONG observed = pp[0] > pp[1] ? pp[0] : pp[1];
            if (observed > peak) {
                InterlockedExchange(&g_pp_peak, observed);
                peak = observed;
            }
            if (pp[0] < peak) {
                uint16_t refill = (uint16_t)peak;
                H->mem_write(save + PP_OFFSET, &refill, sizeof refill);
            }
        }
    }
    if (H->config_bool(mod, "items")) {
        uint8_t bag[BAG_SLOTS * SLOT_SIZE];
        const uint64_t base = save + BAG_OFFSET;
        if (H->mem_read(base, bag, sizeof bag) == MT_OK) {
            /* Reject an unloaded bag or a changed layout before writing any slot. */
            for (unsigned i = 0; i < BAG_SLOTS; ++i) {
                uint64_t vtable;
                memcpy(&vtable, bag + i * SLOT_SIZE, sizeof vtable);
                if (vtable != g_slot_vtable) return MT_EVENT_CONTINUE;
            }
            int target = (int)H->config_number(mod, "item_count");
            if (target < 1) target = 1;
            if (target > 99) target = 99;
            for (unsigned i = 0; i < BAG_SLOTS; ++i) {
                const uint8_t *slot = bag + i * SLOT_SIZE;
                uint32_t packed;
                memcpy(&packed, slot + 8, sizeof packed);
                const uint8_t count = slot[12];
                if ((packed & 0x1fff) && count && count < target) {
                    const uint8_t desired = (uint8_t)target;
                    H->mem_write(base + i * SLOT_SIZE + 12, &desired, 1);
                }
            }
        }
    }
    return MT_EVENT_CONTINUE;
}

static int water_area_count(MtHookCtx *ctx, void *user) {
    MtMod *mod = (MtMod *)user;
    int desired = water_count(mod);
    if (!InterlockedCompareExchange(&g_offline, 0, 0) || !desired) return MT_CONTINUE;
    const uint64_t sp = H->reg_get(ctx, MT_REG_SP);
    uint8_t count = 0;
    if (!sp || H->mem_read(sp + 0x182, &count, 1) != MT_OK || !count) return MT_CONTINUE;
    count = (uint8_t)desired;
    H->mem_write(sp + 0x182, &count, 1);
    return MT_CONTINUE;
}

static int water_cell(MtHookCtx *ctx, void *user) {
    MtMod *mod = (MtMod *)user;
    int desired = water_count(mod);
    if (!InterlockedCompareExchange(&g_offline, 0, 0) || !desired) return MT_CONTINUE;
    const uint64_t sp = H->reg_get(ctx, MT_REG_SP);
    const uint64_t target = H->reg_get(ctx, MT_REG_X0 + 24);
    const uint64_t index = H->reg_get(ctx, MT_REG_X0 + 22);
    if (!sp || !target || index >= (uint64_t)desired) return MT_CONTINUE;
    int16_t coords[3];
    if (H->mem_read(target, &coords[0], 2) != MT_OK ||
        H->mem_read(target + 4, &coords[2], 2) != MT_OK ||
        H->mem_read(sp + 0x4c, &coords[1], 2) != MT_OK) return MT_CONTINUE;
    coords[0] = (int16_t)(coords[0] + g_water_offsets[index][0]);
    coords[2] = (int16_t)(coords[2] + g_water_offsets[index][1]);
    if (H->mem_write(sp + 0x4a, coords, sizeof coords) == MT_OK) {
        H->reg_set(ctx, MT_REG_X0 + 25, sp + 0x4a);
        if (InterlockedCompareExchange(&g_water_seen, 1, 0) == 0)
            mt_logf(H, mod, MT_LOG_INFO, "water gun area set to %d cells", desired);
    }
    return MT_CONTINUE;
}

MT_EXPORT int mod_init(const MtHost *host, MtMod *mod) {
    H = host;
    if (!host->game_build_id || strncmp(host->game_build_id, "76a489f3bd3642fa", 16) != 0)
        return MT_ERR;
    g_save_ptr = host->resolve(mod, "SaveRoot");
    g_game_mgr_ptr = host->resolve(mod, "GameMgr");
    uint64_t main_base = host->module_base("main");
    if (!g_save_ptr || !g_game_mgr_ptr || !main_base) return MT_ERR;
    if (!MT_HOST_HAS(host, key_down) || !host->key_down) return MT_ERR;
    g_slot_vtable = main_base + SLOT_VTABLE_OFFSET;
    build_water_offsets();
    if (host->hook(mod, "water_area_count", water_area_count, mod) != MT_OK ||
        host->hook(mod, "water_cell", water_cell, mod) != MT_OK ||
        host->subscribe(mod, "FrameStart", MT_PRIORITY_NORMAL, on_frame, NULL) != MT_OK)
        return MT_ERR;
    mt_logf(host, mod, MT_LOG_INFO, "personal cheats loaded for Pokopia 2.0.1a");
    return MT_OK;
}
