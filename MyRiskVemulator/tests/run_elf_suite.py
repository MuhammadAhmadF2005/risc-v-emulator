import subprocess
import os
import sys

def run_test(name, path, expected_exit_code, expected_output=None, should_fail_load=False):
    print("========================================")
    print(f" Running Test: {name}")
    print(f" Binary: {path}")
    print("========================================")
    
    rvemu_path = os.path.abspath("MyRiskVemulator/rvemu.exe")
    elf_path = os.path.abspath(path)
    
    res = subprocess.run([rvemu_path, elf_path], capture_output=True, text=True)
    
    print("STDOUT:")
    print(res.stdout.strip())
    if res.stderr.strip():
        print("STDERR:")
        print(res.stderr.strip())
        
    actual_exit = res.returncode
    
    if should_fail_load:
        if actual_exit != 0 and "Failed to load" in res.stderr:
            print("[PASS] Successfully rejected malformed ELF file as expected!")
            print()
            return True
        else:
            print("[FAIL] Emulator did NOT reject malformed ELF file!")
            print()
            return False

    print(f"Exit Code: {actual_exit} (Expected: {expected_exit_code})")
    
    success = True
    if actual_exit != expected_exit_code:
        print(f"[FAIL] Exit code mismatch! Got {actual_exit}, expected {expected_exit_code}")
        success = False
    else:
        print(f"[PASS] Exit code match!")
        
    if expected_output:
        if expected_output in res.stdout:
            print(f"[PASS] Expected output string found!")
        else:
            print(f"[FAIL] Expected output string '{expected_output}' not found in stdout!")
            success = False

    print()
    return success

def main():
    tests = [
        # Basic tests
        ("1..10 Loop Sum", "MyRiskVemulator/tests/bin/loop_sum.elf", 55, None, False),
        ("5! Factorial (MUL)", "MyRiskVemulator/tests/bin/loop_fact.elf", 120, None, False),
        ("Loop Print (sys_write)", "MyRiskVemulator/tests/bin/loop_print.elf", 0, "Loop iteration!\nLoop iteration!\nLoop iteration!", False),
        
        # Extended ELF tests
        ("Multi-Segment (.text + .data + .bss)", "MyRiskVemulator/tests/bin/multi_segment.elf", 100, None, False),
        ("Entry Point Offset + Fibonacci F(10)", "MyRiskVemulator/tests/bin/fibonacci.elf", 55, None, False),
        ("String Output via ELF sys_write", "MyRiskVemulator/tests/bin/hello_str.elf", 0, "RISC-V ELF Loader Verification OK!", False),
        
        # Negative / Error handling tests
        ("Malformed ELF - Invalid Magic", "MyRiskVemulator/tests/bin/bad_magic.elf", 1, None, True),
        ("Malformed ELF - 64-bit Class", "MyRiskVemulator/tests/bin/bad_class64.elf", 1, None, True),
        ("Malformed ELF - Big-Endian Data", "MyRiskVemulator/tests/bin/bad_endian.elf", 1, None, True),
    ]
    
    passed = 0
    total = len(tests)
    
    for name, path, exit_code, output, fail_load in tests:
        if run_test(name, path, exit_code, output, fail_load):
            passed += 1
            
    print("========================================")
    print(f" Full ELF Suite Result: {passed}/{total} Passed")
    print("========================================")
    
    if passed != total:
        sys.exit(1)

if __name__ == "__main__":
    main()
