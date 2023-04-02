#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define PRINT_INFO(MSG, ...) printf("INFO %d %d %s %s %d : " MSG ";;\n", getpid(), getppid(), __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);
#define PRINT_ERROR(MSG, ...) printf("ERROR %d %d %s %s %d : [%d] " MSG ";;\n", getpid(), getppid(), __FILE__, __FUNCTION__, __LINE__,errno, ##__VA_ARGS__);

#define PRINT_ERROR_EXIT(MSG, ...) printf("ERROR %d %d %s %s %d : [%d] " MSG ";;\n", getpid(), getppid(), __FILE__, __FUNCTION__, __LINE__,errno, ##__VA_ARGS__); exit(-1);

int arr[10][10];
int n,a,b,p;
int thapx[10][10];

int isPrime(int x)
{
 
  for(int i=2;i<x;i++)
  {
    if(x%i==0)
      return 0;
  }

  return 1;
}

void *compute_px(void *inp) {


    int x = *((int *) inp);
    int i=x/n;
    int j=x%n;

    // printf("Thread %d of worker process %d started executing\n",j+1,i+1);
    PRINT_INFO("Thread %d of worker process %d started executing\n",j+1,i+1);
    int curr=arr[i][j];

    int sum1=0;
    int px[2*p+1];
    for(int k=curr-1;k>=2;k--)
    {
      if(isPrime(k))
      {
        // printf("Prime number %d discovered\n",k);
        PRINT_INFO("Prime number %d discovered\n",k);
        px[sum1]=k;
        sum1++;
      }
      if(sum1==p)
      break;

    }

    if(isPrime(curr))
    {
      px[sum1]=curr;
      sum1++;
    }

    int sum2=0;
    int k=curr+1;
    while(sum2!=p)
    {
      if(isPrime(k))
      {
        px[sum1]=k;
        sum1++;
        sum2++;
      }
      k++;
    }

    // printf("Set px of element %d(row %d,column %d) created\n",arr[i][j],i,j);
    PRINT_INFO("Set px of element %d(row %d,column %d) created\n",arr[i][j],i,j);
    int currSum=0;
    for(int l=0;l<sum1;l++)
    {
      // printf("PX:%d\n",px[l]);
      currSum+=px[l];
    }

    int currAvg=currSum/sum1; 

    thapx[i][j]=currAvg;
    // printf("Thapx of element %d(row %d,column %d) has been calculated. Value of thapx is %d\n",arr[i][j],i,j,thapx[i][j]);
    PRINT_INFO("Thapx of element %d(row %d,column %d) has been calculated. Value of thapx is %d\n",arr[i][j],i,j,thapx[i][j]);

    free(inp);
}

void handleSignal()
{
  int status;
  int pid1=wait(&status);
  int exitStatus=WEXITSTATUS(status);
  // printf("\n\nExit status=%d\n\n",exitStatus);

  if(exitStatus==1)
  {
    // printf("Invalid input!! Given array elements are not within the range [%d,%d]\nTerminating program",a,b);
    // printf("wpapx of row %d has been written to the controller\n",n);
    kill(0,SIGTERM);
  }
}

int main(int argc, char *argv[])
{

  // printf("Thread %d of worker process %d started executing\n",j+1,i+1);
  // PRINT_INFO("Thread %d of worker process %d started executing\n", 5, 6);
  signal(SIGCHLD,handleSignal);
  if(argc<=5)
  {
    // printf("Invalid input. Insufficient number of arguments passed!\n");
    PRINT_ERROR("Invalid input. Insufficient number of arguments passed!\n");
    return 0;
  }


  // int n,a,b,p;
  n=atoi(argv[1]),a=atoi(argv[2]),b=atoi(argv[3]),p=atoi(argv[4]);

  if(argc!=n*n+5)
  {
    // printf("Invalid input. Insufficient number of arguments passed!\n");
    PRINT_ERROR("Invalid input. Insufficient number of arguments passed!\n");
    return 0;
  }

  // int arr[n][n];

  for(int i=0;i<n*n;i++)
  {
    int x=i/n;
    int y=i%n;
    arr[x][y]=atoi(argv[5+i]);
  }

  PRINT_INFO("\nInput read\n");
  PRINT_INFO("Value of n=%d\n",n);
  PRINT_INFO("Value of a=%d\n",a);
  PRINT_INFO("Value of b=%d\n",b);
  PRINT_INFO("Value of p=%d\n\n",p);

  PRINT_INFO("Array elements: ");
  for(int i=0;i<n;i++)
  {
    for(int j=0;j<n;j++)
    printf("%d ",arr[i][j]);
  }
  printf("\n\n");

  int fd[n][2];  //file descriptor for pipe
  for(int i=0;i<n;i++)
  {
    pipe(fd[i]);
  }

  printf("%d pipes created\n\n",n);

  for(int i=0;i<n;i++)
  {
    pid_t pid=fork();
    if(pid>0)
    {
      PRINT_INFO("Worker process %d created\n",i+1);
    }
    else if(pid==0)
    {   
        // child process
        // CHECK if all values are between  a and b
        PRINT_INFO("\nProcessing row %d\n",i);


        for(int j=0;j<n;j++)
        {
          if(arr[i][j]<a||arr[i][j]>b)
          {
            PRINT_ERROR_EXIT("Invalid input!! Given array elements are not within the range [%d,%d]\nTerminating program",a,b");
          }
        }

        pthread_t tid[n];
        for(int j=0;j<n;j++)
        {
          int *arg = malloc(sizeof(*arg));

          *arg = i*n+j;
          pthread_create(&tid[j], NULL, compute_px, arg);
          PRINT_INFO("Thread %d of worker process %d created\n",j+1,i+1);
        }


        for(int j=0;j<n;j++)
        {
          pthread_join(tid[j], NULL);
          PRINT_INFO("Thread %d of worker process %d joined\n",j+1,i+1);
        }

        PRINT_INFO("\nAll(%d) values of thapx have been calculated of row %d\n\n",n,i);

        int currSum=0;

        for(int j=0;j<n;j++)
        {
          // printf("THAPX:%d\n",thapx[i][j]);
          currSum+=thapx[i][j];
        }

        int wpapx=currSum/n;
        PRINT_INFO("wpapx of row %d has been calculated. Value of wpapx is %d\n",i,wpapx);

        close(fd[i][0]);  //closing the read end of the pipe

        // send the value on the write-descriptor.
        write(fd[i][1], &wpapx, sizeof(wpapx));
        PRINT_INFO("wpapx of row %d has been written to the controller\n",n);

        // close the write descriptor
        close(fd[i][1]);
        // sleep(19);

        exit(0);
    }
  }
  
  for(int i=0;i<1000;i++)
  fflush(stdout);

  sleep(1000);
  for(int i=0;i<n;i++) 
  {
    wait(NULL);
  }  

  int wSum=0;

  for(int i=0;i<n;i++)
  {
    //closing the write end of the pipe
    close(fd[i][1]);

    int curr;

    read(fd[i][0], &curr, sizeof(curr));
    PRINT_INFO("wpapx of row %d has been captured by the controller. Value is %d\n",n,curr);
    wSum+=curr;
    // printf("CURR:%d\n",curr);
    

    close(fd[i][0]);
  }
  
  int fapx=wSum/n;
  PRINT_INFO("\nfapx has been calculated by the controller\n");
  PRINT_INFO("Value of fapx = %d\n",fapx);
  // printf("%d",fapx);
  

  return 0;
}