/************
Instructions To run this code:
Run ./server from bash Before running client
************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <time.h>

double power(double,double);
struct my_msgbuf {
    long mtype;
    char mtext[100];
};

int main(void)
{
    struct my_msgbuf buf;
    int msqid;
    key_t key;
    char buffer[30];
    time_t timer;
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    /*Date and Time Generation*/
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S:", tm_info);
    /*Opening A file In Append Mode*/
    FILE *fp = fopen("result.txt","a");
    fprintf(fp,"%s \n",buffer);
    fclose(fp);
    /*Creating Unique Key same for both clinet and server*/
    if ((key = ftok("server.c", 'S')) == -1) {
        perror("ftok");
        exit(1);
    }
    /*Connecting To the queue*/
    if ((msqid = msgget(key, 0644)) == -1) { 
        perror("msgget");
        exit(1);
    }
    
    printf("server: ready to receive command\n");
    int i,j,k,var,opr;
    double op[2];
    for(;;) {
        for(i=1;i<=10;i++)
        {
            /*Receving Message from the message queue*/
            var=msgrcv(msqid, &buf, sizeof(buf.mtext),i, IPC_NOWAIT);
            j=0,k=0,op[0]=0,op[1]=0;
            char *ins = buf.mtext,opr;
            /*Getting the the required operands
             in op[0] and op[1] and operator opr*/
            while(*(ins+j)!='\0'&&var!=-1)
            {
                if(*(ins+j)==' '){j++;}
                else if(*(ins+j)=='e'){op[k]=2.71828;k++;j++;}
                else
                {
                    if(47<*(ins+j)&&*(ins+j)<58)
                    {
                        while(47<*(ins+j)&&*(ins+j)<58)
                        {
                            op[k] = op[k]*10 + *(ins+j)-48;
                            j++;
                        }
                        k++;
                    }
                    else
                    {
                        opr = *(ins+j);
                        j++;
                    }
                }
            }
            if (var!= -1) 
            {
                /*Computing the Result*/
                if(opr == '+') op[0]=op[0]+op[1];
                else if(opr == '-') op[0]=op[0]-op[1];
                else if(opr == '*') op[0]=op[0]*op[1];
                else if(opr == '^') op[0]=power(op[0],op[1]);
                else op[0]=0;
                /*Writing to the file as well ass scree*/
                printf("Server: Client %d \"%s\" = %f Written To file\n",i,buf.mtext,op[0]);
                fp = fopen("result.txt","a");
                fprintf(fp,"Client %d: \"%s\" = %f\n",i,buf.mtext,op[0]);
                fclose(fp);   
            }
        }
    }
    return 0;
}
/*Efficinet way to calculate power*/
double power(double x,double y)
{
    double res=1;
    while(y>0)
    {
        if((int)y%2!=0) res = res * x;
        x = x*x;
        y=y/2; 
    }
    return res;
}