#define DELAY_IN_WAIT 1000000

//////////////////////////////////////////////////////////////
// to guarantee, dst should be longer than end-start+1
void string_copy(char *dst, const char *src, int start, int end) {
  int i = start;
  while (i <= end) {
    dst[i-start] = src[i++];
  }
  dst[i] = '\0';
}
