#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>

#include "debug.h"
#include "helper.h"
#include "mailbox.h"


typedef struct mailbox{
    char *handle;
    ENTRY *head;
    ENTRY *tail;
    MAILBOX_DISCARD_HOOK *hook;
    volatile int ref_count;
    volatile int defunct;
    sem_t mutex;
    sem_t mailbox_cnt;
}MAILBOX;

MAILBOX *mb_init(char *handle){
    MAILBOX *mailbox=calloc(1,sizeof(MAILBOX));
    mailbox->handle=malloc(sizeof(strlen(handle)+1));
    strcpy(mailbox->handle,handle);
    mailbox->ref_count=1;
    mailbox->defunct=0;
    sem_init(&mailbox->mutex,0,1);
    sem_init(&mailbox->mailbox_cnt,0,0);
    // mailbox->head=calloc(1,sizeof(ENTRY));
    // mailbox->tail=calloc(1,sizeof(ENTRY));
    debug("Initalizing mailbox for %s",mailbox->handle);
    return mailbox;
}

void mb_set_discard_hook(MAILBOX *mb, MAILBOX_DISCARD_HOOK *hook){
    sem_wait(&mb->mutex);
    mb->hook=hook;
    sem_post(&mb->mutex);
}

void mb_ref(MAILBOX *mb, char *why){
    sem_wait(&mb->mutex);
    mb->ref_count+=1;
    debug("Increase reference to %i:  %s",mb->ref_count,why);
    sem_post(&mb->mutex);
}

void mb_unref(MAILBOX *mb, char *why){
    sem_wait(&mb->mutex);
    mb->ref_count-=1;
    debug("Unreferencing mailbox to %i:   %s",mb->ref_count,why);
    if(mb->ref_count==0){
        debug("Freeing Mailbox");
        while(mb->head!=NULL){
            ENTRY *temp=mb->head;
            mb->head=mb->head->next;
            (mb->hook)(temp->entry);
            free(temp->entry);
            free(temp);
        }
        sem_post(&mb->mutex);
        sem_destroy(&mb->mailbox_cnt);
        sem_destroy(&mb->mutex);
        free(mb->handle);
        free(mb);
        return;
    }
    sem_post(&mb->mutex);
}

void mb_shutdown(MAILBOX *mb){
    sem_wait(&mb->mutex);
    mb->defunct=1;
    sem_post(&mb->mutex); 
    sem_post(&mb->mailbox_cnt);  
}

char *mb_get_handle(MAILBOX *mb){
    sem_wait(&mb->mutex);
    char *handle=mb->handle;
    sem_post(&mb->mutex);
    return handle;
}

void mb_add_message(MAILBOX *mb, int msgid, MAILBOX *from, void *body, int length){
    if(mb!=from){
        mb_ref(from,"Increasing reference of sender");
    }
    sem_wait(&mb->mutex);
    MAILBOX_ENTRY *new_mentry=malloc(sizeof(MAILBOX_ENTRY));
    new_mentry->type=MESSAGE_ENTRY_TYPE;
    new_mentry->content.message.body=body;
    new_mentry->content.message.from=from;
    new_mentry->content.message.length=length;
    new_mentry->content.message.msgid=msgid;
    if(mb->head==NULL){
        //List is empty
        mb->head=calloc(1,sizeof(ENTRY));
        mb->head->entry=new_mentry;
        mb->head->next=NULL;
        mb->head->prev=NULL;
        mb->tail=mb->head;
    }else{
        ENTRY *new_entry=malloc(sizeof(ENTRY));
        new_entry->entry=new_mentry;
        new_entry->prev=mb->tail;
        new_entry->next=NULL;
        mb->tail->next=new_entry;
        mb->tail=new_entry;
    }
    sem_post(&mb->mutex);
    sem_post(&mb->mailbox_cnt);
}

void mb_add_notice(MAILBOX *mb, NOTICE_TYPE ntype, int msgid){
    sem_wait(&mb->mutex);
    MAILBOX_ENTRY *new_mentry=malloc(sizeof(MAILBOX_ENTRY));
    new_mentry->type=NOTICE_ENTRY_TYPE;
    new_mentry->content.notice.msgid=msgid;
    new_mentry->content.notice.type=ntype;
    if(mb->head==NULL){
        mb->head=calloc(1,sizeof(ENTRY));
        mb->head->entry=new_mentry;
        mb->head->next=NULL;
        mb->head->prev=NULL;
        mb->tail=mb->head;
    }else{
        ENTRY *new_entry=malloc(sizeof(ENTRY));
        new_entry->entry=new_mentry;
        new_entry->prev=mb->tail;
        new_entry->next=NULL;
        mb->tail->next=new_entry;
        mb->tail=new_entry;
    }
    sem_post(&mb->mutex);
    sem_post(&mb->mailbox_cnt);
}

MAILBOX_ENTRY *mb_next_entry(MAILBOX *mb){
    sem_wait(&mb->mailbox_cnt);
    if(mb->defunct==1){
        sem_post(&mb->mailbox_cnt);
        return NULL;
    }
    sem_wait(&mb->mutex);
    MAILBOX_ENTRY *entry=mb->head->entry;
    if(mb->head->entry!=mb->tail->entry){
        ENTRY *temp=mb->head;
        mb->head=mb->head->next;
        free(temp);
    }else{
        free(mb->head);
        mb->head=NULL;
        mb->tail=NULL;
    }
    sem_post(&mb->mutex);
    if(entry->type==MESSAGE_ENTRY_TYPE){
        mb_unref(mb,"ENTRY TYPE UNREF");
    }
    return entry;
}