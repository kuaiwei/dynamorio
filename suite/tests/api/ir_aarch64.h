/* **********************************************************
 * Copyright (c) 2015-2022 Google, Inc.  All rights reserved.
 * Copyright (c) 2022 ARM Limited. All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of ARM Limited nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL ARM LIMITED OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

static byte buf[8192];

#ifdef STANDALONE_DECODER
#    define ASSERT(x)                                                                 \
        ((void)((!(x)) ? (fprintf(stderr, "ASSERT FAILURE (standalone): %s:%d: %s\n", \
                                  __FILE__, __LINE__, #x),                            \
                          abort(), 0)                                                 \
                       : 0))
#else
#    define ASSERT(x)                                                                \
        ((void)((!(x)) ? (dr_fprintf(STDERR, "ASSERT FAILURE (client): %s:%d: %s\n", \
                                     __FILE__, __LINE__, #x),                        \
                          dr_abort(), 0)                                             \
                       : 0))
#endif

#define TEST_INSTR(instruction_name) \
    void test_instr_##instruction_name(void *dc, instr_t *instr, bool *psuccess)

#define RUN_INSTR_TEST(instruction_name)                          \
    test_result = true;                                           \
    test_instr_##instruction_name(dcontext, instr, &test_result); \
    if (test_result == false) {                                   \
        print("test for " #instruction_name " failed.\n");        \
        result = false;                                           \
    }

#define TEST_NO_OPNDS(opcode, create_name, expected)            \
    instr = INSTR_CREATE_##create_name(dc);                     \
    if (!test_instr_encoding(dc, OP_##opcode, instr, expected)) \
        *psuccess = false;

#define TEST_LOOP(opcode, create_name, number, expected, args...)   \
    for (int i = 0; i < number; i++) {                              \
        instr = INSTR_CREATE_##create_name(dc, args);               \
        if (!test_instr_encoding(dc, OP_##opcode, instr, expected)) \
            *psuccess = false;                                      \
    }

static bool
test_instr_encoding(void *dc, uint opcode, instr_t *instr, const char *expected)
{
    instr_t *decin;
    byte *pc;
    char *end, *big;
    size_t len = strlen(expected);
    size_t buflen = (len < 100 ? 100 : len) + 2;
    bool result = true;
    char *buf = malloc(buflen);

    if (instr_get_opcode(instr) != opcode) {
        print("incorrect opcode for instr %s: %s\n\n", decode_opcode_name(opcode),
              decode_opcode_name(instr_get_opcode(instr)));
        instr_destroy(dc, instr);
        return false;
    }
    instr_disassemble_to_buffer(dc, instr, buf, buflen);
    end = buf + strlen(buf);
    if (end > buf && *(end - 1) == '\n')
        --end;

    if (!instr_is_encoding_possible(instr)) {
        print("encoding for expected %s not possible\n", expected);
        instr_destroy(dc, instr);
        return false;
    }

    if (end - buf != len || memcmp(buf, expected, len) != 0) {
        print("dissassembled as:\n");
        print("   %s\n", buf);
        print("but expected:\n");
        print("   %s\n\n", expected);
        instr_destroy(dc, instr);
        return false;
    }

    pc = instr_encode(dc, instr, buf);
    decin = instr_create(dc);
    decode(dc, buf, decin);
    if (!instr_same(instr, decin)) {
        print("Reencoding failed, dissassembled as:\n   ");
        instr_disassemble(dc, decin, STDERR);
        print("\n");
        print("but expected:\n");
        print("   %s\n", expected);
        print("Encoded as:\n");
        print("   0x%08x\n\n", *((int *)pc));
        result = false;
    }

    instr_destroy(dc, instr);
    instr_destroy(dc, decin);

    return result;
}

const reg_id_t Zn_six_offset_0[6] = { DR_REG_Z0,  DR_REG_Z5,  DR_REG_Z10,
                                      DR_REG_Z16, DR_REG_Z21, DR_REG_Z31 };
const reg_id_t Zn_six_offset_1[6] = { DR_REG_Z0,  DR_REG_Z6,  DR_REG_Z11,
                                      DR_REG_Z17, DR_REG_Z22, DR_REG_Z31 };
const reg_id_t Zn_six_offset_2[6] = { DR_REG_Z0,  DR_REG_Z7,  DR_REG_Z12,
                                      DR_REG_Z18, DR_REG_Z23, DR_REG_Z31 };
const reg_id_t Zn_six_offset_3[6] = { DR_REG_Z0,  DR_REG_Z8,  DR_REG_Z13,
                                      DR_REG_Z19, DR_REG_Z24, DR_REG_Z31 };
const reg_id_t Pn_half_six_offset_0[6] = { DR_REG_P0, DR_REG_P2, DR_REG_P3,
                                           DR_REG_P5, DR_REG_P6, DR_REG_P7 };
const reg_id_t Pn_six_offset_0[6] = { DR_REG_P0, DR_REG_P2,  DR_REG_P5,
                                      DR_REG_P8, DR_REG_P10, DR_REG_P15 };
const reg_id_t Pn_six_offset_1[6] = { DR_REG_P0, DR_REG_P3,  DR_REG_P6,
                                      DR_REG_P9, DR_REG_P11, DR_REG_P15 };
const reg_id_t Pn_six_offset_2[6] = { DR_REG_P0,  DR_REG_P4,  DR_REG_P7,
                                      DR_REG_P10, DR_REG_P12, DR_REG_P15 };
const reg_id_t Xn_six_offset_0[6] = { DR_REG_X0,  DR_REG_X5,  DR_REG_X10,
                                      DR_REG_X15, DR_REG_X20, DR_REG_X30 };
const reg_id_t Wn_six_offset_0[6] = { DR_REG_W0,  DR_REG_W5,  DR_REG_W10,
                                      DR_REG_W15, DR_REG_W20, DR_REG_W30 };
