#ifndef COLORS_H
#define COLORS_H

// text colors
#define RESET   "\033[0m"
#define BLACK   "\033[0;30m"
#define RED     "\033[0;31m"
#define GREEN   "\033[38;2;119;221;119m"    // Pastel green
#define YELLOW  "\033[38;2;253;253;150m"    // Pastel yellow
#define BLUE    "\033[38;2;0;102;255m"    // Vivid blue (not pastel)
#define MAGENTA "\033[38;2;255;179;255m"    // Pastel magenta
#define CYAN    "\033[38;2;178;255;255m"    // Pastel cyan
#define WHITE   "\033[0;37m"
#define PINK         "\033[1;35m"             // Bright magenta as pink
#define ROSE_PINK    "\033[38;2;255;102;204m" // RGB(255, 102, 204)
#define PASTEL_PINK  "\033[38;2;255;182;193m" // RGB(255, 182, 193)

// light text colors
#define LIGHT_BLACK   "\033[0;90m"
#define LIGHT_RED     "\033[0;91m"
#define LIGHT_GREEN   "\033[0;92m"
#define LIGHT_YELLOW  "\033[0;93m"
#define LIGHT_BLUE    "\033[0;94m"
#define LIGHT_MAGENTA "\033[0;95m"
#define LIGHT_CYAN    "\033[0;96m"
#define LIGHT_WHITE   "\033[0;97m"

// bold text colors
#define BOLD_BLACK   "\033[1;30m"
#define BOLD_RED     "\033[1;31m"
#define BOLD_GREEN   "\033[1;32m"
#define BOLD_YELLOW  "\033[1;33m"
#define BOLD_BLUE    "\033[1;34m"
#define BOLD_MAGENTA "\033[1;35m"
#define BOLD_CYAN    "\033[1;36m"
#define BOLD_WHITE   "\033[1;37m"

// background colors
#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"

// light background colors
#define BG_LIGHT_BLACK   "\033[0;100m"
#define BG_LIGHT_RED     "\033[0;101m"
#define BG_LIGHT_GREEN   "\033[0;102m"
#define BG_LIGHT_YELLOW  "\033[0;103m"
#define BG_LIGHT_BLUE    "\033[0;104m"
#define BG_LIGHT_MAGENTA "\033[0;105m"
#define BG_LIGHT_CYAN    "\033[0;106m"
#define BG_LIGHT_WHITE   "\033[0;107m"

#endif // COLORS_H