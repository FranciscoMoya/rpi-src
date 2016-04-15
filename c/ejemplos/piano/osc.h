#define ARGS(x) args_##x x

typedef struct { } args_quit;

typedef struct { int receive; } args_notify;

// no implementamos el mensaje opcional
typedef struct {
    char data[256];
    int size;
} args_d_recv;

// no implementamos el mensaje opcional
typedef struct {
    char* path;
} args_d_load;

// no implementamos el mensaje opcional
typedef struct {
    char* path;
} args_d_loadDir;

// solo implementamos un argumento
typedef struct {
    int id;
} args_n_free;

// simplificamos el formato, solo 
typedef struct {
    const char* name;
    int value;
} params_s_new;

#define MAX_S_NEW_PARAMS 4
typedef struct {
    const char* name;
    int id;
    int add_action;
    int add_target;
    params_s_new param[MAX_S_NEW_PARAMS];
} args_s_new;


typedef struct {
    const char* cmd;
    const char* format;
    int async;
    union {
	ARGS(quit);
	ARGS(notify);
	
	ARGS(d_recv);
	ARGS(d_load);
	ARGS(d_loadDir);

	ARGS(n_free);
	ARGS(s_new);

	ARGS(done);
	ARGS(fail);
	ARGS(n_go);
	ARGS(n_end);
	ARGS(n_off);
	ARGS(n_on);
	ARGS(n_move);
	ARGS(tr);
    } args;
} osc_message;
