/* rf_core.c  (freestanding / no libc / no includes)
   - núcleo: ciclo ψ→χ→ρ→Δ→Σ→Ω→ψ
   - BitStack append-only
   - CRC32 bitwise
*/
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

#define BITSTACK_CAP 4096u

static u32 crc32_bitwise(const u8 *data, u32 len) {
  u32 crc = 0xFFFFFFFFu;
  u32 i, j;
  for (i = 0; i < len; i++) {
    crc ^= (u32)data[i];
    for (j = 0; j < 8u; j++) {
      u32 m = (u32)-(int)(crc & 1u);
      crc = (crc >> 1) ^ (0xEDB88320u & m);
    }
  }
  return ~crc;
}

struct BitStack {
  u8  buf[BITSTACK_CAP];
  u32 w;
  u32 records;
  u32 last_crc;
};

static void bs_init(struct BitStack *bs) {
  bs->w = 0u;
  bs->records = 0u;
  bs->last_crc = 0u;
}

static u32 bs_push(struct BitStack *bs, const u8 *data, u32 len) {
  if (!bs) return 0u;
  if (len > 0xFFFFu) return 0u;

  u32 need = 2u + 4u + len;
  if (bs->w + need > BITSTACK_CAP) return 0u;

  u32 crc = crc32_bitwise(data, len);

  bs->buf[bs->w + 0u] = (u8)(len & 0xFFu);
  bs->buf[bs->w + 1u] = (u8)((len >> 8) & 0xFFu);

  bs->buf[bs->w + 2u] = (u8)(crc & 0xFFu);
  bs->buf[bs->w + 3u] = (u8)((crc >> 8) & 0xFFu);
  bs->buf[bs->w + 4u] = (u8)((crc >> 16) & 0xFFu);
  bs->buf[bs->w + 5u] = (u8)((crc >> 24) & 0xFFu);

  for (u32 i = 0; i < len; i++) bs->buf[bs->w + 6u + i] = data[i];

  bs->w += need;
  bs->records += 1u;

  /* witness rolling */
  {
    u8 tmp[10];
    tmp[0] = (u8)(bs->last_crc & 0xFFu);
    tmp[1] = (u8)((bs->last_crc >> 8) & 0xFFu);
    tmp[2] = (u8)((bs->last_crc >> 16) & 0xFFu);
    tmp[3] = (u8)((bs->last_crc >> 24) & 0xFFu);
    tmp[4] = (u8)(crc & 0xFFu);
    tmp[5] = (u8)((crc >> 8) & 0xFFu);
    tmp[6] = (u8)((crc >> 16) & 0xFFu);
    tmp[7] = (u8)((crc >> 24) & 0xFFu);
    tmp[8] = (u8)(len & 0xFFu);
    tmp[9] = (u8)((len >> 8) & 0xFFu);
    bs->last_crc = crc32_bitwise(tmp, 10u);
  }

  return crc;
}

enum RafaState {
  ST_PSI = 0,
  ST_CHI = 1,
  ST_RHO = 2,
  ST_DEL = 3,
  ST_SIG = 4,
  ST_OMG = 5
};

struct RafaCore {
  u64 t;
  u32 s;
  u32 coherence;
  u32 entropy;
  u32 ethica;
  u32 last_in_crc;
  u32 last_out_crc;
  struct BitStack log;
};

static u32 clamp_u32(u32 x, u32 lo, u32 hi) {
  if (x < lo) return lo;
  if (x > hi) return hi;
  return x;
}

static u32 phi_ethica(u32 coherence, u32 entropy, u32 ethica) {
  u64 c = (u64)coherence;
  u64 e = (u64)(1000000000u - clamp_u32(entropy, 0u, 1000000000u));
  u64 score = (c * e) / 1000000000ull;
  if (score > (u64)ethica) score = (u64)ethica;
  return (u32)score;
}

static void step_delta(struct RafaCore *rc) {
  if (rc->entropy > 700000000u) {
    rc->ethica = clamp_u32(rc->ethica + 1000000u, 0u, 1000000000u);
    if (rc->coherence > 5000000u) rc->coherence -= 5000000u;
  } else {
    rc->coherence = clamp_u32(rc->coherence + 3000000u, 0u, 1000000000u);
    if (rc->entropy > 1000000u) rc->entropy -= 1000000u;
  }
}

static void commit_sigma(struct RafaCore *rc, const u8 *payload, u32 len) {
  u8 frame[64];
  u32 p = 0u;

  frame[p++] = (u8)'R';
  frame[p++] = (u8)'F';

  frame[p++] = (u8)(rc->t & 0xFFu);
  frame[p++] = (u8)((rc->t >> 8) & 0xFFu);
  frame[p++] = (u8)((rc->t >> 16) & 0xFFu);
  frame[p++] = (u8)((rc->t >> 24) & 0xFFu);
  frame[p++] = (u8)((rc->t >> 32) & 0xFFu);
  frame[p++] = (u8)((rc->t >> 40) & 0xFFu);
  frame[p++] = (u8)((rc->t >> 48) & 0xFFu);
  frame[p++] = (u8)((rc->t >> 56) & 0xFFu);

  frame[p++] = (u8)(rc->s & 0xFFu);

  {
    u32 v, k;
    v = rc->coherence; for (k=0;k<4u;k++) frame[p++] = (u8)((v >> (8u*k)) & 0xFFu);
    v = rc->entropy;   for (k=0;k<4u;k++) frame[p++] = (u8)((v >> (8u*k)) & 0xFFu);
    v = rc->ethica;    for (k=0;k<4u;k++) frame[p++] = (u8)((v >> (8u*k)) & 0xFFu);
  }

  if (len > 0xFFFFu) len = 0xFFFFu;
  frame[p++] = (u8)(len & 0xFFu);
  frame[p++] = (u8)((len >> 8) & 0xFFu);

  (void)bs_push(&rc->log, frame, p);

  if (payload && len) {
    rc->last_in_crc = crc32_bitwise(payload, len);
    (void)bs_push(&rc->log, payload, len);
  }
}

static void rafa_init(struct RafaCore *rc) {
  rc->t = 0ull;
  rc->s = ST_PSI;
  rc->coherence = 500000000u;
  rc->entropy   = 500000000u;
  rc->ethica    = 800000000u;
  rc->last_in_crc = 0u;
  rc->last_out_crc = 0u;
  bs_init(&rc->log);
}

static void rafa_tick(struct RafaCore *rc, const u8 *in, u32 in_len, u8 *out, u32 out_cap, u32 *out_len) {
  rc->t++;

  rc->s = ST_CHI;
  u32 in_crc = 0u;
  if (in && in_len) in_crc = crc32_bitwise(in, in_len);

  rc->s = ST_RHO;
  {
    u32 x = (rc->log.last_crc ^ in_crc);
    u32 n = 0u;
    while (x) { x &= (x - 1u); n++; }
    u32 delta = n * 20000000u;
    rc->entropy = clamp_u32(rc->entropy + delta, 0u, 1000000000u);
  }

  rc->s = ST_DEL;
  step_delta(rc);

  u32 score = phi_ethica(rc->coherence, rc->entropy, rc->ethica);

  rc->s = ST_SIG;
  commit_sigma(rc, in, in_len);

  rc->s = ST_OMG;
  if (out && out_cap >= 20u) {
    u32 p = 0u;
    out[p++] = (u8)'O'; out[p++] = (u8)'M'; out[p++] = (u8)'G'; out[p++] = (u8)'A';

    out[p++] = (u8)(score & 0xFFu);
    out[p++] = (u8)((score >> 8) & 0xFFu);
    out[p++] = (u8)((score >> 16) & 0xFFu);
    out[p++] = (u8)((score >> 24) & 0xFFu);

    out[p++] = (u8)(rc->log.last_crc & 0xFFu);
    out[p++] = (u8)((rc->log.last_crc >> 8) & 0xFFu);
    out[p++] = (u8)((rc->log.last_crc >> 16) & 0xFFu);
    out[p++] = (u8)((rc->log.last_crc >> 24) & 0xFFu);

    out[p++] = (u8)(rc->log.records & 0xFFu);
    out[p++] = (u8)((rc->log.records >> 8) & 0xFFu);
    out[p++] = (u8)((rc->log.records >> 16) & 0xFFu);
    out[p++] = (u8)((rc->log.records >> 24) & 0xFFu);

    out[p++] = (u8)(rc->s & 0xFFu);
    out[p++] = 0u; out[p++] = 0u; out[p++] = 0u;

    rc->last_out_crc = crc32_bitwise(out, p);
    if (out_len) *out_len = p;
  } else {
    if (out_len) *out_len = 0u;
  }

  rc->s = ST_PSI;
}

static struct RafaCore g_rc;

void rafa_boot(void) {
  rafa_init(&g_rc);
}

u32 rafa_run(const u8 *in, u32 in_len, u8 *out, u32 out_cap) {
  u32 out_len = 0u;
  rafa_tick(&g_rc, in, in_len, out, out_cap, &out_len);
  return out_len;
}
