.text
.global count_det

count_det:
    push   {r4-r9}

    ldr    r5, [r0], #4
    neg    r4, r5
    ldr    r1, [r0], #4
    neg    r6, r1
    ldr    r3, [r0], #4
    neg    r2, r3
    
    ldr    r7, [r0], #4  
    mul    r3, r7
    mul    r6, r7
    ldr    r7, [r0], #4  
    mul    r5, r7
    mul    r2, r7
    ldr    r8, [r0], #4 
    mul    r1, r7
    mul    r4, r7
    
    ldr    r7, [r0], #4  
    mul    r1, r7
    mla    r2, r7, r2, r1
    ldr    r7, [r0], #4 
    mla    r3, r7, r3, r2
    mla    r4, r7, r4, r3
    ldr    r7, [r0], #4  
    mla    r5, r7, r5, r4
    mla    r6, r7, r6, r5
    
    mov    r0, r6
    bx     lr
    pop    {r4-r9}
