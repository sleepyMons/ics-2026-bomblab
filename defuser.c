#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t low = (uint32_t)challenge;
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t step2 = 3U * (high - low) + UINT32_C(0x4ef2);
    uint32_t step0 = 2U * low + ((uint32_t)(challenge >> 16) ^ UINT32_C(0xda66));

    values[0] = (5U * step0) ^ (step2 + UINT32_C(0x35e1));
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    values[0] = (uint32_t)((challenge >> 40) & UINT64_C(0x1f));
    for (uint32_t i = 0; i < 5; ++i) {
        uint32_t shift = 11 + 2 * i;
        uint32_t nibble = (uint32_t)((challenge >> shift) & UINT64_C(0xf));
        values[i + 1] = (nibble + values[i]) ^ (15 * (i + 1));
    }
    size_t count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t index = (uint32_t)(challenge & UINT64_C(7));
    uint32_t input = (uint32_t)((challenge >> 3) ^ (challenge >> 32));
    uint32_t result;

    values[0] = index;
    switch (index) {
    case 0:
        result = (input ^ UINT32_C(0x964f)) + (input >> 2);
        break;
    case 1:
        result = 5U * input + UINT32_C(0xc5b7);
        break;
    case 2:
        result = (2U * input) ^ UINT32_C(0xf130);
        break;
    case 3:
        result = 5U * input + UINT32_C(0x4d788);
        break;
    case 4:
        result = 3U * input + UINT32_C(0xc2c4);
        break;
    case 5:
        result = (2U * input) ^ UINT32_C(0x2627);
        break;
    case 6:
        result = 5U * input + UINT32_C(0x2d71);
        break;
    default:
        result = (input ^ UINT32_C(0xe7f)) + (input >> 1);
        break;
    }

    values[1] = result;
    size_t count = 2;
    return write_answer(out, cap, values, count);
}

static uint32_t phase4_walk(uint32_t target, uint32_t low, uint32_t high, uint32_t salt) {
    uint32_t mid = low + ((high - low) >> 1);
    if (target == mid) return (mid + UINT32_C(0xb5)) ^ salt;

    uint32_t path_salt = mid + salt;
    uint32_t result;
    if (target < mid) {
        result = phase4_walk(target, low, mid - 1, salt);
        return (3U * result ^ path_salt) ^ UINT32_C(2);
    }

    result = phase4_walk(target, mid + 1, high, salt);
    return (3U * result ^ path_salt) ^ UINT32_C(0x1d);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t target = (uint32_t)(challenge & UINT64_C(0x7f));
    uint32_t salt = (uint32_t)((challenge >> 7) ^ (challenge >> 32));

    values[0] = phase4_walk(target, 0, UINT32_C(0x7f), salt);
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    static const uint32_t node_values[12] = {
        0x0581, 0x0a56, 0x0767, 0x0885, 0x0471, 0x0666,
        0x0b0d, 0x0937, 0x030b, 0x001e, 0x0263, 0x017a
    };
    static const uint32_t node_salts[12] = {
        0xf19d, 0xb0ea, 0xa880, 0x73da, 0xb498, 0xe584,
        0xc32a, 0x0545, 0xc921, 0xafdd, 0xac01, 0x5b68
    };
    static const uint32_t next_nodes[12] = {
        7, 11, 12, 6, 0, 9, 8, 10, 1, 3, 5, 2
    };
    static const uint32_t head_nodes[8] = {4, 0, 7, 10, 5, 9, 3, 6};

    uint32_t node = head_nodes[(uint32_t)(challenge & UINT64_C(7))];
    uint32_t hops = (uint32_t)((challenge >> 3) & UINT64_C(3));
    uint32_t accumulator = (uint32_t)((challenge >> 6) ^ (challenge >> 32));
    accumulator = 5U * ((accumulator ^ UINT32_C(0x30a9)) + node_salts[node])
                ^ node_values[node];
    values[1] = node_values[node];

    for (uint32_t i = 0; i < hops; ++i) {
        node = next_nodes[node];
        accumulator = 5U * (accumulator + node_salts[node]) ^ node_values[node];
        values[1] = node_values[node];
    }

    values[0] = accumulator;
    size_t count = 2;
    return write_answer(out, cap, values, count);
}

static uint32_t phase6_op(uint32_t index, uint32_t value) {
    switch (index) {
    case 0:
        return (value ^ UINT32_C(0x285b)) + 8U * value;
    case 1:
        return ((value >> 3) ^ value) + UINT32_C(0x176b);
    case 2:
        return (value + UINT32_C(0xa0f2)) ^ (value >> 1);
    case 3:
        return ((value << 2) | (value >> 30)) ^ UINT32_C(0x1ad6);
    case 4:
        return (8U * value + UINT32_C(0x6b58)) ^ (7U * value);
    case 5:
        return (value + UINT32_C(0xa0f2)) ^ (value >> 1);
    case 6:
        return ((value << 2) | (value >> 30)) ^ UINT32_C(0x1ad6);
    default:
        return (value >> 2) | (value << 30);
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t first_index = (uint32_t)(challenge & UINT64_C(7));
    uint32_t second_index = (uint32_t)((challenge >> 3) & UINT64_C(7));
    uint32_t input = (uint32_t)((challenge >> 6) ^ (challenge >> 32));
    uint32_t intermediate = phase6_op(first_index, input) ^ UINT32_C(0xdab8);

    values[0] = phase6_op(second_index, intermediate) + UINT32_C(0x495b);
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t first = (uint32_t)context & UINT32_C(3);
    uint32_t challenge_mix = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t context_mix = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t mixed_input = challenge_mix ^ context_mix;
    uint32_t second = (5U * mixed_input + UINT32_C(0x145f))
                    ^ UINT32_C(0xad48)
                    ^ (first * UINT32_C(0x145f))
                    ^ (uint32_t)(context >> 16);
    uint32_t third = second
                   ^ (context_mix + first + UINT32_C(0xad48)
                      + (challenge_mix >> 3));
    int size = snprintf(out, cap, "%-32sOPEN %" PRIu32 " %" PRIu32 " %" PRIu32,
                        "", first, second, third);
    return size < 0 || (size_t)size >= cap;
}
