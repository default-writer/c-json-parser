#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

static void skip_ws(const char **s) {
    while (isspace((unsigned char)**s)) (*s)++;
}

static bool match_literal(const char **s, const char *lit) {
    size_t n = strlen(lit);
    if (strncmp(*s, lit, n) == 0) {
        *s += n;
        return true;
    }
    return false;
}

static bool parse_string(const char **s) {
    if (**s != '"') return false;
    (*s)++;
    while (**s && **s != '"') {
        if ((unsigned char)**s < 0x20) return false; // control chars not allowed
        if (**s == '\\') {
            (*s)++;
            if (**s == '\0') return false;
            char c = **s;
            if (c == 'u') {
                // expect 4 hex digits
                (*s)++;
                for (int i = 0; i < 4; ++i) {
                    char h = **s;
                    if (!isxdigit((unsigned char)h)) return false;
                    (*s)++;
                }
                continue;
            }
            // allowed escapes: " \ / b f n r t
            if (c == '"' || c == '\\' || c == '/' || c == 'b' ||
                c == 'f' || c == 'n' || c == 'r' || c == 't') {
                (*s)++;
                continue;
            }
            return false;
        } else {
            (*s)++;
        }
    }
    if (**s != '"') return false;
    (*s)++;
    return true;
}

static bool parse_number(const char **s) {
    const char *p = *s;
    if (*p == '-') p++;
    if (*p == '0') {
        p++;
    } else if (isdigit((unsigned char)*p)) {
        while (isdigit((unsigned char)*p)) p++;
    } else {
        return false;
    }
    if (*p == '.') {
        p++;
        if (!isdigit((unsigned char)*p)) return false;
        while (isdigit((unsigned char)*p)) p++;
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') p++;
        if (!isdigit((unsigned char)*p)) return false;
        while (isdigit((unsigned char)*p)) p++;
    }
    *s = p;
    return true;
}

static bool parse_value(const char **s);

static bool parse_array(const char **s) {
    if (**s != '[') return false;
    (*s)++;
    skip_ws(s);
    if (**s == ']') { (*s)++; return true; }
    while (1) {
        skip_ws(s);
        if (!parse_value(s)) return false;
        skip_ws(s);
        if (**s == ',') { (*s)++; continue; }
        if (**s == ']') { (*s)++; return true; }
        return false;
    }
}

static bool parse_object(const char **s) {
    if (**s != '{') return false;
    (*s)++;
    skip_ws(s);
    if (**s == '}') { (*s)++; return true; }
    while (1) {
        skip_ws(s);
        if (!parse_string(s)) return false;
        skip_ws(s);
        if (**s != ':') return false;
        (*s)++;
        skip_ws(s);
        if (!parse_value(s)) return false;
        skip_ws(s);
        if (**s == ',') { (*s)++; continue; }
        if (**s == '}') { (*s)++; return true; }
        return false;
    }
}

static bool parse_value(const char **s) {
    skip_ws(s);
    if (**s == '"') return parse_string(s);
    if (**s == '{') return parse_object(s);
    if (**s == '[') return parse_array(s);
    if (**s == 'n') return match_literal(s, "null");
    if (**s == 't') return match_literal(s, "true");
    if (**s == 'f') return match_literal(s, "false");
    if (**s == '-' || isdigit((unsigned char)**s)) return parse_number(s);
    return false;
}

// Public function requested: takes const char* and returns bool true/false
bool func(const char *json) {
    if (!json) return false;
    const char *p = json;
    skip_ws(&p);
    if (!parse_value(&p)) return false;
    skip_ws(&p);
    return *p == '\0';
}