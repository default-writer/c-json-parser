static bool whitespace_lookup[256] = {0};

__attribute__((constructor)) static void init_whitespace_lookup(void) {
  ((bool *)whitespace_lookup)[' '] = true;
  ((bool *)whitespace_lookup)['\t'] = true;
  ((bool *)whitespace_lookup)['\n'] = true;
  ((bool *)whitespace_lookup)['\r'] = true;
}