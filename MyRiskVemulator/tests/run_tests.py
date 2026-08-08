import subprocess
import os
import sys

def run_test(name, path, expected_exit_code, expected_output=None):
    print(f"========================================")
    print(f" Running Test: {name}")
    print(f" Binary: {path}")
    print(f"========================================")
    
    rvemu_path = os.path.abspath("MyRiskVemulator/rvemu.exe")
    elf_path = os.path.abspath(path)
    
    res = subprocess.run([rvemu_path, elf_path], capture_output=True, text=True)
    
    print("STDOUT:")
    print(res.stdout.strip())
    if res.stderr.strip():
        print("STDERR:")
        print(res.stderr.strip())
        
    actual_exit = res.returncode
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
        ("1..10 Loop Sum", "MyRiskVemulator/tests/bin/loop_sum.elf", 55, None),
        ("5! Factorial (MUL)", "MyRiskVemulator/tests/bin/loop_fact.elf", 120, None),
        ("Loop Print (sys_write)", "MyRiskVemulator/tests/bin/loop_print.elf", 0, "Loop iteration!\nLoop iteration!\nLoop iteration!")
    ]
    
    passed = 0
    total = len(tests)
    
    for name, path, exit_code, output in tests:
        if run_test(name, path, exit_code, output):
            passed += 1
            
    print(f"========================================")
    print(f" Test Summary: {passed}/{total} Passed")
    print(f"========================================")
    
    if passed != total:
        sys.exit(1)

if __name__ == "__main__":
    main()
