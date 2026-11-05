#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <spawn.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#include "solver.h"

extern char **environ;

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
  #define PATH_SEP ";"
  #define DIR_SEP  "\\"
  #define EXE_NAME "frame3dd.exe"
  #define strdup _strdup
  #define stat _stat
  #define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#else
  #include <unistd.h>
  #include <dirent.h>
  #define PATH_SEP ":"
  #define DIR_SEP  "/"
  #define EXE_NAME "frame3dd"
#endif

// regular file (not a folder) that can be run
static int is_runnable(const char* p) {
    struct stat st;
    if (stat(p, &st) != 0 || !S_ISREG(st.st_mode)) return 0;
#ifdef _WIN32
    return 1;
#else
    return access(p, X_OK) == 0;
#endif
}

// folder containing the viewer's own executable
static int exe_dir(char* out, size_t n) {
#ifdef _WIN32
    DWORD k = GetModuleFileNameA(NULL, out, (DWORD)n);
    if (k == 0 || k >= n) return 0;
#else
    ssize_t k = readlink("/proc/self/exe", out, n - 1);
    if (k <= 0) return 0;
    out[k] = 0;
#endif
    char* s = strrchr(out, DIR_SEP[0]);
    if (!s) return 0;
    *s = 0;
    return 1;
}

static void parent_of(char* path) {
    char* s = strrchr(path, DIR_SEP[0]);
    if (s) *s = 0;
}

static int try_file(const char* dir, const char* rel, char* out, size_t n) {
    snprintf(out, n, "%s%s%s%s", dir, DIR_SEP, rel, EXE_NAME);
    return is_runnable(out);
}

#ifndef _WIN32
// look for the executable in dir and in its subfolders, up to 'depth' levels
static int search_tree(const char* dir, int depth, char* out, size_t n) {
    snprintf(out, n, "%s/%s", dir, EXE_NAME);
    if (is_runnable(out)) return 1;
    if (depth <= 0) return 0;

    DIR* d = opendir(dir);
    if (!d) return 0;
    struct dirent* e;
    while ((e = readdir(d)) != NULL) {
        if (e->d_name[0] == '.') continue;               // skip hidden folders and . ..
        char sub[1024];
        snprintf(sub, sizeof sub, "%s/%s", dir, e->d_name);
        struct stat st;
        if (stat(sub, &st) != 0 || !S_ISDIR(st.st_mode)) continue;
        if (search_tree(sub, depth - 1, out, n)) { closedir(d); return 1; }
    }
    closedir(d);
    return 0;
}
#endif

// returns 1 and fills 'out' with the full path if found, else 0
int find_frame3dd(const char* user_path, char* out, size_t n) {
    out[0] = 0;

    // 1. path saved or chosen by the user
    if (user_path && *user_path && is_runnable(user_path)) {
        snprintf(out, n, "%s", user_path);
        return 1;
    }

    // 2. next to the viewer (bundled with it)
    char dir[1024];
    int have_dir = exe_dir(dir, sizeof dir);
    if (have_dir) {
        static const char* rel[] = { "", "bin" DIR_SEP, "frame3dd" DIR_SEP };
        for (int i = 0; i < 3; i++)
            if (try_file(dir, rel[i], out, n)) return 1;
    }

    // 3. PATH (works if the user installed it normally)
    const char* path = getenv("PATH");
    if (path) {
        char* copy = strdup(path);
        for (char* d = strtok(copy, PATH_SEP); d; d = strtok(NULL, PATH_SEP)) {
            snprintf(out, n, "%s%s%s", d, DIR_SEP, EXE_NAME);
            if (is_runnable(out)) { free(copy); return 1; }
        }
        free(copy);
    }

    // 4. common install locations
    static const char* common[] = {
#ifdef _WIN32
        "C:\\Program Files\\Frame3DD", "C:\\Program Files (x86)\\Frame3DD", "C:\\Frame3DD",
#else
        "/usr/local/bin", "/usr/bin", "/opt/frame3dd", "/opt/frame3dd/bin",
#endif
    };
    for (size_t i = 0; i < sizeof common / sizeof common[0]; i++) {
        snprintf(out, n, "%s%s%s", common[i], DIR_SEP, EXE_NAME);
        if (is_runnable(out)) return 1;
    }

#ifndef _WIN32
    // 5. search the folders around the viewer (finds ../frame3dd/ in a dev layout)
    if (have_dir) {
        char parent[1024];
        snprintf(parent, sizeof parent, "%s", dir);
        parent_of(parent);
        if (parent[0] && search_tree(parent, 3, out, n)) return 1;
    }
    const char* home = getenv("HOME");
    if (home) {
        char p[1024];
        snprintf(p, sizeof p, "%s/.local/bin", home);
        if (search_tree(p, 0, out, n)) return 1;
    }
#endif

    out[0] = 0;
    return 0;
}



int run_solver(const char *solver, const char *model, char *outDir, size_t n) {
    char tmpl[] = "/tmp/f3dview_XXXXXX";
    if (!mkdtemp(tmpl)) return -1;                       // fresh empty folder for this run
    snprintf(outDir, n, "%s", tmpl);

    char out[1100], err[1100];
    snprintf(out, sizeof out, "%s/model.out", outDir);   // results (the solver appends here)
    snprintf(err, sizeof err, "%s/stderr.txt", outDir);  // error messages

    setenv("FRAME3DD_OUTDIR", outDir, 1);                // extra files (.plt, meshes) go here

    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_addopen(&fa, 2, err, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    char *argv[] = { (char*)solver, "-i", (char*)model, "-o", out, "-q", NULL };
    pid_t pid;
    int rc = posix_spawn(&pid, solver, &fa, NULL, argv, environ);
    posix_spawn_file_actions_destroy(&fa);
    if (rc != 0) return -1;

    int status;
    waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : -2;
}
