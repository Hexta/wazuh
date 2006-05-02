/*   $OSSEC, main.c, v0.4, 2005/11/09, Daniel B. Cid$   */

/* Copyright (C) 2003,2004,2005 Daniel B. Cid <dcid@ossec.net>
 * All right reserved.
 *
 * This program is a free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */



#include "shared.h"
#include "remoted.h"


int main(int argc, char **argv)
{
    int i = 0,c = 0;
    int uid = 0, gid = 0;
    int test_config = 0;
    
    char *cfg = DEFAULTCPATH;
    char *dir = DEFAULTDIR;
    char *user = REMUSER;
    char *group = GROUPGLOBAL;

    
    /* Setting the name -- must be done ASAP */
    OS_SetName(ARGV0);

    
    while((c = getopt(argc, argv, "dthu:g:D:")) != -1){
        switch(c){
            case 'h':
                help();
                break;
            case 'd':
                nowDebug();
                break;
            case 'u':
                if(!optarg)
                    ErrorExit("%s: -u needs an argument",ARGV0);
                user = optarg;
                break;
            case 'g':
                if(!optarg)
                    ErrorExit("%s: -g needs an argument",ARGV0);
                group = optarg;
                break;		
            case 't':
                test_config = 1;    
                break;
            case 'D':
                if(!optarg)
                    ErrorExit("%s: -D needs an argument",ARGV0);
                dir = optarg;
        }
    }

    debug1(STARTED_MSG,ARGV0);
    
    
    /* Return 0 if not configured */
    if(RemotedConfig(cfg, &logr) < 0)
    {
        ErrorExit(CONFIG_ERROR,ARGV0);
    }


    /* Exit if test_config is set */
    if(test_config)
        exit(0);

        
    /* Check if the user/group given are valid */
    uid = Privsep_GetUser(user);
    gid = Privsep_GetGroup(group);
    if((uid < 0)||(gid < 0))
        ErrorExit(USER_ERROR,user,group);


    /* Going on daemon */
    nowDaemon();
    goDaemon();

    
    /* Setting new group */
    if(Privsep_SetGroup(gid) < 0)
            ErrorExit(SETGID_ERROR,ARGV0,gid);

    /* Going on chroot */
    if(Privsep_Chroot(dir) < 0)
                ErrorExit(CHROOT_ERROR,ARGV0,dir);


    nowChroot();


    /* Starting the signal manipulation */
    StartSIG(ARGV0);	


    i = getpid();


    
    /* Creating some randoness  */
    srand( time(0) + getpid()+ i);
    rand();
    

    /* Start up message */
    verbose(STARTUP_MSG, ARGV0, getpid());

    /* Really starting the program. */
    i = 0; 
    while(logr.conn[i] != 0)
    {
        /* Forking for each connection handler */
        if(fork() == 0)
        {   
            /* On the child */
            HandleRemote(i, uid);
        }
        else
        {
            i++;
            continue;
        }
    }


    /* Done over here */
    return(0);
}


/* EOF */
