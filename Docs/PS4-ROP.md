# **PS4 Browser, ROP, and Gadgets**

## **Summary**
The PS4 browser's reliance on WebKit and the system's security mechanisms, like DEP and ASLR, make Return-Oriented Programming a key technique for exploitation. By extracting gadgets from loaded modules (`libkernel`, `libSceLibcInternal`, `libSceWebKit2`), attackers can build ROP chains to achieve arbitrary code execution, escape the sandbox, and load custom payloads.

## **PS4 Browser and WebKit**

The PS4 browser uses the **WebKit** engine, an open-source layout engine responsible for rendering web pages. WebKit is also used in other devices such as iOS, Wii U, and PS Vita, but its vulnerabilities have made it a crucial entry point for exploiting the PS4.

### **WebKit Vulnerabilities**
- The PS4 browser in firmware 1.76 uses a version of WebKit vulnerable to **CVE-2012-3748**, a heap-based buffer overflow in the `JSArray::sort(...)` method.
- This vulnerability allows attackers to:
  - Achieve arbitrary read/write access to memory within the WebKit process.
  - Overwrite return addresses on the stack, gaining control over the instruction pointer register (`rip`).
  
This exploit was initially ported to the PS4 by nas and Proxima, who demonstrated its effectiveness and released a proof of concept (PoC).

### **PS4 Sandbox and Workarounds**
The browser operates within a sandboxed environment (the `WebProcess.self` process), restricting its capabilities. To interact with the system beyond the sandbox, an attacker uses techniques like ROP to escape these limitations.

- If the Internet Browser is unavailable, the "User's Guide" option in the settings menu provides a limited web browser interface that can be manipulated via a proxy.

---

## **Understanding ROP (Return-Oriented Programming)**

### **What is ROP?**
> [See ROP Syntax Examples](./ROP_Syntax.md)

Return-Oriented Programming is an advanced exploitation technique that allows attackers to execute arbitrary code without injecting new code into memory. Instead, it chains together small snippets of existing code called **gadgets**, which end in a `ret` instruction.

- **Purpose of ROP**: 
  - Circumvent security mechanisms like Data Execution Prevention (DEP), which prevents execution of writable memory.
  - Manipulate the program's control flow to achieve the attacker's goals.

### **How ROP Works**
1. **Control the Stack**: Exploits like buffer overflows are used to overwrite the stack and control the return address.
2. **Find Gadgets**: Gadgets are located in executable memory regions of loaded modules.
3. **Chain Gadgets**: Gadgets are chained together to perform desired actions, such as invoking system calls or modifying memory.

### **Example of ROP Execution**
Consider the following gadgets in memory:
- At `0x80000`: `mov rax, 0; ret`
- At `0x90000`: `mov rbx, 0; ret`

If the stack contains `0x80000` followed by `0x90000`, the following occurs:
- Execution jumps to `0x80000` and sets `rax` to 0.
- The `ret` instruction pops `0x90000` off the stack, jumping to `mov rbx, 0`.

This effectively sets `rax` and `rbx` to 0, despite not injecting new code.

---

## **Gadgets and Their Role**

### **What Are Gadgets?**
Gadgets are small sequences of instructions that exist in the program's code and end in a `ret`. These snippets perform specific tasks (e.g., loading a value into a register or adding two numbers).

- **Example Gadget**: `pop rdi; ret`
  - Pops a value from the stack into the `rdi` register and returns control to the next address on the stack.

### **Using Gadgets in ROP Chains**
- Gadgets are chained by carefully arranging values on the stack.
- Each `ret` instruction in a gadget transfers control to the next gadget.

#### **Complex Gadget Usage**
Some gadgets may perform multiple operations, such as:
```
pop r8
pop r9
ret
```
- In this case, dummy values must be provided for `r9` if only `r8` needs to be set.

### **Finding Gadgets**
- Gadgets are found in memory regions marked as executable.
- Tools like `ROPgadget` or `rp++` are used to automate the search for gadgets by scanning binaries and memory dumps.
- Gadgets can also be discovered in unintended code paths by decoding instructions starting at unexpected byte offsets.

---

## **Extracting Gadgets from PS4 Modules**

The PS4 browser process (`WebProcess.self`) loads several essential system modules into memory, making them the primary source of gadgets for exploitation.

### **Key Modules for Gadget Extraction**
1. **`libkernel.sprx`**:
   - Provides low-level system calls and kernel interactions.
   - Critical for invoking system-level functions, privilege escalation, and memory management.

2. **`libSceLibcInternal.sprx`**:
   - Contains standard library functions for memory management and I/O.
   - Offers a variety of general-purpose gadgets.

3. **`libSceWebKit2.sprx`**:
   - WebKit engine module loaded by the PS4 browser.
   - A rich source of gadgets due to its extensive functionality.

### **Memory Dump Sizes**
- **`libSceWebKit2.sprx`**: ~26 MB
- **`libkernel.sprx`**: ~287 KB
- **`libSceLibcInternal.sprx`**: ~1.17 MB

### **Steps to Extract Gadgets**
1. **Dump Decrypted Memory**:
   - Memory is dumped during runtime, as modules are encrypted at rest.
   - The dumped files (e.g., `webkit.bin`, `libkernel.bin`) are raw binary data.

2. **Reconstruct ELF Files**:
   - Convert raw binaries into ELF files (Executable and Linkable Format) for easier analysis.

3. **Use Gadget Discovery Tools**:
   - Tools like `ROPgadget` scan ELF files for instruction sequences ending in `ret`, generating gadget lists (e.g., `webkit-gadgets.txt`).

### **Challenges**
- **ASLR**: Address Space Layout Randomization randomizes the base address of modules in memory, complicating gadget chaining.
  - This is mitigated by leaking memory addresses or reading the module table to calculate offsets.
