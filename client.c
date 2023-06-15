
#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //socket
#include <arpa/inet.h> //inet_addr

int candidateCount = 0;
char candidates[50][50];

int initialiseCandidatesList(){
  FILE* candidateFile = fopen("Candidates_List.txt", "r");

  if (candidateFile == NULL)
  {
    printf("Voters List file not found!\n");
    return 0;
  }

  for (int i = 0; i < 50; i++)
  {
    memset(candidates[i],'\0',sizeof(candidates[i]));
  }

  int i = 0;
  while(fgets(candidates[i], 50, candidateFile) != NULL)
  {
    printf(candidates[i]);
    i++;
  }

  candidateCount = i;

  fclose(candidateFile);

  return 1;
}

void displayCandidates(){
  for (int i = 0; i < candidateCount; i++)
  {
    printf(candidates[i]);
  }
}

int main(void)
{
  initialiseCandidatesList();

  int socket_desc;
  struct sockaddr_in server_addr;
  char server_message[2000], client_message[2000];

  //Cleaning the Buffers

  memset(server_message,'\0',sizeof(server_message));
  memset(client_message,'\0',sizeof(client_message));

  //Creating Socket

  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if(socket_desc < 0)
  {
    printf("Could Not Create Socket. Error!!!!!\n");
    return -1;
  }

  printf("Socket Created\n");

  //Specifying the IP and Port of the server to connect

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(2000);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  //Now connecting to the server accept() using connect() from client side

  if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
  {
    printf("Connection Failed. Error!!!!!");
    return -1;
  }

  printf("Connected\n");

  //Get Name and CNIC from the Client side
  printf("Enter Message: ");
  gets(client_message);

  //Send Client's message to the Server
  if(send(socket_desc, client_message, strlen(client_message),0) < 0)
  {
    printf("Send Failed. Error!!!!\n");
    return -1;
  }

  //Receive the server's response
  if(recv(socket_desc, server_message, sizeof(server_message),0) < 0)
  {
    printf("Receive Failed. Error!!!!!\n");
    return -1;
  }

  printf("Server Message: %s\n",server_message);

  //strcmp returns 0 when two strings are equal
  if (strcmp(server_message, "Already Voted!") == 0)
  {
    //do nothing
  }
  else
  {
    if (strcmp(server_message, "Voter Not Found!") == 0)
    {
      printf("Please restart client and enter correct details!\n");
    }

    //Voter was found in list and had not voted
    else
    {
      if (strcmp(server_message, "Server Full!") == 0)
      {
        //do nothing
      }
      else
      {
        memset(server_message,'\0',sizeof(server_message));
        memset(client_message,'\0',sizeof(client_message));

        printf("\nCandidates:\n");
        displayCandidates();
        printf("\n");

        printf("Enter Symbol to vote for:\n");
        gets(client_message);

        //Send Client's message to the Server
        if(send(socket_desc, client_message, strlen(client_message),0) < 0)
        {
          printf("Send Failed. Error!!!!\n");
          return -1;
        }

        //Receive the server's response
        if(recv(socket_desc, server_message, sizeof(server_message),0) < 0)
        {
          printf("Receive Failed. Error!!!!!\n");
          return -1;
        }

        printf("Server Message: %s\n",server_message);
      }

    }
  }

  //clean buffers
  memset(server_message,'\0',sizeof(server_message));
  memset(client_message,'\0',sizeof(client_message));

  //Closing the Socket
  close(socket_desc);

  printf("\n");

  return 0;
}
