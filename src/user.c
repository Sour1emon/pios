#include "user.h"
#include "user_sys.h"

void loop(char *str) {
  char buf[2] = {""};
  while (1) {
    for (int i = 0; i < 5; i++) {
      buf[0] = str[i];
      call_sys_write(buf);
      user_delay(1000000);
    }
  }
}

void user_process() {
  call_sys_write("User process\n\r");

  // Touch multiple stack pages to trigger faults
  volatile char *p1 = (char *)0x1000; // Page 1
  volatile char *p2 = (char *)0x2000; // Page 2
  volatile char *p3 = (char *)0x3000; // Page 3 - should fail if ind limit hits

  *p1 = 'A';
  call_sys_write("Touched page 1\n\r");
  *p2 = 'B';
  call_sys_write("Touched page 2\n\r");
  *p3 = 'C'; // This should trigger the 3rd fault
  call_sys_write(
      "Touched page 3\n\r"); // If you see this, the limit didn't kill you

  int pid = call_sys_fork();
  if (pid < 0) {
    call_sys_write("Error during fork\n\r");
    call_sys_exit();
    return;
  }
  if (pid == 0) {
    loop("abcde");
  } else {
    loop("12345");
  }
}
