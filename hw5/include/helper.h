#include <stdio.h>
#include <stdlib.h>

#include "mailbox.h"

typedef struct entry ENTRY;

typedef struct entry{
    MAILBOX_ENTRY * entry;
    ENTRY *prev;
    ENTRY *next;
}ENTRY;

