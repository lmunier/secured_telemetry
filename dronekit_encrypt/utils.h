struct param_struct {
    int iterations;
    char semaphore_name[512];
    char shared_memory_name[512];
    int permissions;
    int size;
};


void say(const char *, const char *);
int acquire_semaphore(const char *, sem_t *);
int release_semaphore(const char *, sem_t *);
void read_params(struct param_struct *);
