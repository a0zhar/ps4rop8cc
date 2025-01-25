# **Assembly-like ROP Format**

The toolchain provides a flexible and assembly-like syntax for constructing ROP chains, making it easier to manipulate gadgets and build complex exploits. Below is a comprehensive breakdown of the syntax, with examples and explanations for each command.

---

### **1. Labels**

**Syntax**: `<label>:`

Labels are used to define specific locations in the ROP chain. They act as named anchors that can be referenced later in the script (e.g., for jumps or offsets). Labels must be valid Python identifiers (alphanumeric characters and underscores, starting with a letter).

**Example**:
```asm
start:
pop rdi
dq 0x12345678  # Load 0x12345678 into the rdi register
```

---

### **2. Assembly Instructions**

**Syntax**: `<asm_instr>`

This command searches for a gadget in the loaded memory dumps that matches the specified assembly instruction (`asm_instr`) followed by a `ret`. If the gadget is found, its address is inserted into the ROP chain. If the gadget is not found, an error is raised.

Gadgets can be located in the `dumps/gadgets.txt` file after dumping the memory.

**Example**:
```asm
pop rdi
pop rsi
mov rax, rdi
```
If these gadgets are available in the dumps, the tool will find their addresses and insert them into the ROP chain.

---

### **3. Comments**

**Syntax**: `# comment`

Any text following the `#` symbol on a line is treated as a comment and ignored by the toolchain. Comments are useful for documenting the ROP chain or explaining specific operations.

**Example**:
```asm
# Set up rdi register with 0x12345678
pop rdi
dq 0x12345678
```

---

### **4. Define Bytes (`db`)**

**Syntax**: `db <expr>`

This command writes raw bytes into the ROP chain. The `<expr>` must evaluate to a collection of integers (values in the range 0–255), which are then directly written as bytes.

**Examples**:
```asm
db 1, 2, 3               # Writes bytes: 0x01, 0x02, 0x03
db bytes([65, 66, 67])   # Writes ASCII 'ABC' (0x41, 0x42, 0x43)
db [0xff, 0x00, 0xaa]    # Writes: 0xFF, 0x00, 0xAA
```

---

### **5. Define Quadwords (`dq`)**

**Syntax**: `dq <expr>`

The `<expr>` must evaluate to an integer. This command writes the integer as a 64-bit little-endian value into the ROP chain.

**Examples**:
```asm
dq 0x123456789abcdef0    # Writes: F0 DE BC 9A 78 56 34 12
dq 0                    # Writes a NULL quadword: 00 00 00 00 00 00 00 00
```

---

### **6. Define Pointer (`dp`)**

**Syntax**: `dp <expr>`

This is similar to `dq`, but the `<expr>` must evaluate to an offset relative to the start of the ROP chain. The absolute address of the offset is then calculated and written.

This is particularly useful for creating jumps to labeled sections within the ROP chain.

**Example**:
```asm
pop rsp                 # Prepare to jump
dp label_to_jump_to     # Write the absolute address of 'label_to_jump_to'
label_to_jump_to:
pop rdi
dq 0x12345678
```

---

### **7. JavaScript Expressions (`$<expr>`)**

**Syntax**: `$<expr>`

The `<expr>` is a JavaScript expression evaluated in the exploited WebKit context. The result is written as a 64-bit little-endian value into the ROP chain. This is used to dynamically compute addresses during exploitation.

**Note**: JavaScript numbers are stored as 64-bit floating-point values (IEEE 754 format), which limits their precision to 52 bits. Be cautious when handling large integers.

**Example**:
```asm
$some_js_variable  # Insert the value of a JavaScript variable
```

**Usage Example**:
```javascript
// WebKit Context Example
let some_js_variable = 0xdeadbeef;
```
In the ROP script:
```python
pop rdi
$some_js_variable  # Dynamically insert the value of 'some_js_variable'
```

---

### **8. Literal JavaScript (`$$<expr>`)**

**Syntax**: `$$<expr>`

This inserts `<expr>` directly into the generated JavaScript code. It is useful for defining custom JavaScript functions or logic within the exploit.

If `<expr>` is exactly `pivot(ropchain);`, the tool omits the final pivot statement.

**Example**:
```asm
$$function(myParam) {
$$    console.log(myParam);
$$    pivot(ropchain);
$$}
```
