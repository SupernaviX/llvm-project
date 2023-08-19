    .text
    .global __memcpy_wordaligned
    .type   __memcpy_wordaligned,@function
__memcpy_wordaligned:
    /* r6 is dest, r7 is src, r8 is count. */
    /* r8 is assumed to be both nonzero and a multiple of 4. */
    st.w r6, -4[r3]
.Lmemcpy_loop:
    ld.w 0[r7], r10
    st.w r10, 0[r6]
    add 4, r6
    add 4, r7
    add -4, r8
    bnz .Lmemcpy_loop
    ld.w -4[r3], r10
    jmp [r31]
.Lmemcpy_end:
    .size	__memcpy_wordaligned, .Lmemcpy_end-__memcpy_wordaligned
