
void test_sdcard();
extern bool did_post_sdcard;

enum sdcard_result {OK=0, FAIL_INIT, FAIL_MOUNT, FAIL_WRITE, FAIL_MKDIR, FAIL_CREATE, FAIL_CLOSE, FAIL_UNKNOWN};
enum sdcard_result write_log_to_sdcard();
