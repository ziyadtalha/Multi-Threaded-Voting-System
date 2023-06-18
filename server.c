#include <stdio.h>
#include <string.h>
#include <sys/socket.h> //socket
#include <arpa/inet.h> //inet_addr
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//number of client currently connected using the threads
int clientCount = 0;
pthread_t threads[5];

int voterCount = 0;
char voters[50][50];

int initialiseVoters(){
  FILE* voterFile = fopen("Voters_List.txt", "r");

  if (voterFile == NULL)
  {
    printf("Voters List file not found!\n");
    return -1;
  }

  for (int i = 0; i < 50; i++)
  {
    memset(voters[i],'\0',sizeof(voters[i]));
  }

  int i = 0;
  while(fgets(voters[i], 50, voterFile) != NULL)
  {
    i++;
  }

  voterCount = i;

  fclose(voterFile);

  return 1;
}

void displayVoters(){
  for (int i = 0; i < voterCount; i++)
  {
    printf(voters[i]);
  }
}

int candidateCount = 0;
char candidates[50][50];

int initialiseCandidatesList(){
  FILE* candidateFile = fopen("Candidates_List.txt", "r");

  if (candidateFile == NULL)
  {
    printf("Voters List file not found!\n");
    return -1;
  }

  for (int i = 0; i < 50; i++)
  {
    memset(candidates[i],'\0',sizeof(candidates[i]));
  }

  int i = 0;
  while(fgets(candidates[i], 50, candidateFile) != NULL)
  {
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

char symbols[50][50];

void initialiseSymbols(){
  for (int i = 0; i < 50; i++)
  {
    memset(symbols[i],'\0',sizeof(symbols[i]));
  }

  for (int i = 0; i < candidateCount; i++)
  {
    char temp[50];
    memset(temp,'\0',sizeof(temp));

    int canLen = strlen(candidates[i]);
    int x = canLen - 2;
    int symbolLength = 0;

    while ((candidates[i][x] != ' ') && (x > 0))
    {
      temp[symbolLength] = candidates[i][x];
      x--;
      symbolLength++;
    }

    for (int k = 0, j = (symbolLength - 1); k < symbolLength; k++, j--)
    {
      symbols[i][k] = temp[j];
    }
  }
}


//records client details and candidate symbol into a file
void vote(char* cd, char* cs)
{
  FILE* votes = fopen("Votes.txt", "a");

  if (votes == NULL)
  {
    printf("Votes file could not be opened!\n");
    return -1;
  }

  fputs(cs, votes);
  fputs("/", votes);
  fputs(cd, votes);

  fclose(votes);
}


//here we build a thread function to service a client
void *server_thread (void *args)
{
  char server_message[2000], client_details[2000], client_symbol[2000];                 // Sending values from the server and receive from the server we need this

  //Cleaning the Buffers
  memset(server_message,'\0',sizeof(server_message));
  memset(client_details,'\0',sizeof(client_details)); // Set all bits of the padding field//
  memset(client_symbol,'\0',sizeof(client_symbol));

	int newSocket = *((int *)args);

	int clientNum = clientCount;

	clientCount++;

  //Receive the message from the client

  if (recv(newSocket, client_details, sizeof(client_details),0) < 0)
  {
    printf("Receive Failed. Error!!!!!\n");
    return -1;
  }

  printf("Client Message: %s\n", client_details);

  //need to do this otherwise check will not be correct
  strcat(client_details, "\n");

  //check if client details are valid or not

  int cd_len = strlen(client_details);

  int foundFlag = 0;

  for (int i = 0; i < voterCount; i++)
  {
    //we only do string comparison if length of client message and the string at current index in voters list array are the same
    if (cd_len == strlen(voters[i]))
    {
      //compare if strings are the same or not!
      //strcmp was not giving intended output here because of garbage values I think!
      for (int j = 0; j < cd_len; j++)
      {
        //we stop comparison as soon as a single character is different
        if (client_details[j] != voters[i][j])
        {
          break;
        }
      }

      foundFlag = 1;
      break;
    }
  }

  memset(server_message,'\0',sizeof(server_message));

  //Deciding which message to send to client
  if (foundFlag == 1)
  {
    strcpy(server_message, "Enter Voter Details!");
  }
  else
  {
    strcpy(server_message, "Voter Not Found!");
  }

  if (send(newSocket, server_message, strlen(server_message),0) < 0)
  {
    printf("Send Failed. Error!!!!!\n");
    return -1;
  }

  //if client was found, then receive candidate symbol from them
  if (foundFlag == 1)
  {
    if (recv(newSocket, client_symbol, sizeof(client_symbol),0) < 0)
    {
      printf("Receive Failed. Error!!!!!\n");
      return -1;
    }

    printf("Client Sent Symbol: %s\n", client_symbol);
    strcat(client_symbol, "\n");

    int cs_len = strlen(client_symbol);

    //check if symbol exists
    int exists = 0;

    for (int i = 0; i < candidateCount; i++)
    {
      if (strlen(symbols[i]) == cs_len)
      {
        for (int j = 0; i < cs_len; i++)
        {
          if (symbols[i][j] != client_symbol[j])
          {
            exists = 0;
            break;
          }
        }

        exists = 1;
      }
    }

    //if client exists then we vote
    if (exists == 1)
    {
      strcpy(server_message, "Voted!");

      vote(client_details, client_symbol);
    }
    else
    {
      strcpy(server_message, "Symbol not found!");
    }

    if (send(newSocket, server_message, strlen(server_message),0) < 0)
    {
      printf("Send Failed. Error!!!!!\n");
      return -1;
    }

  }

  memset(server_message,'\0',sizeof(server_message));
  memset(client_details,'\0',sizeof(client_details));
  memset(client_symbol,'\0',sizeof(client_symbol));

  //Closing the Socket
  close(newSocket);

  clientCount--;

  return NULL;
}


int main(void)
{
  if(initialiseVoters() == -1)
  {
    return -1;
  }

  if (initialiseCandidatesList() == -1)
  {
    return -2;
  }

  initialiseSymbols();

  int socket_desc, client_sock, client_size;
  struct sockaddr_in server_addr, client_addr;         //SERVER ADDR will have all the server address
  char server_message[2000], client_details[2000], client_symbol[2000];                 // Sending values from the server and receive from the server we need this

  //Cleaning the Buffers
  memset(server_message,'\0',sizeof(server_message));
  memset(client_details,'\0',sizeof(client_details));     // Set all bits of the padding field//
  memset(client_symbol,'\0',sizeof(client_symbol));

  //Creating Socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if(socket_desc < 0)
  {
    printf("Could Not Create Socket. Error!!!!!\n");
    return -3;
  }
  printf("Socket Created\n");

  //Binding IP and Port to socket

  server_addr.sin_family = AF_INET;               /* Address family = Internet */
  server_addr.sin_port = htons(2000);               // Set port number, using htons function to use proper byte order */
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");    /* Set IP address to localhost */

  // BINDING FUNCTION

  if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr))<0)    // Bind the address struct to the socket.  /
    //bind() passes file descriptor, the address structure,and the length of the address structure
  {
    printf("Bind Failed. Error!!!!!\n");
    return -4;
  }
  printf("Bind Done\n");

  //while loop allows us to listen for a new client connection after a client disconnects
  while (1 == 1)
  {
    //Put the socket into Listening State
    if(listen(socket_desc, 1) < 0)                               //This listen() call tells the socket to listen to the incoming connections.
    // The listen() function places all incoming connection into a "backlog queue" until accept() call accepts the connection.
    {
      printf("Listening Failed. Error!!!!!\n");
      return -5;
    }

    printf("Listening for Incoming Connections.....\n");

    //Accept the incoming Connections

    client_size = sizeof(client_addr);

    client_sock = accept(socket_desc, (struct sockaddr*)&client_addr, &client_size);          // heree particular client k liye new socket create kr rhaa ha

    if (client_sock < 0)
    {
      printf("Accept Failed. Error!!!!!!\n");
      return -6;
    }

    printf("Client Connected with IP: %s and Port No: %i\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
    //inet_ntoa() function converts the Internet host address in, given in network byte order, to a string in IPv4 dotted-decimal notation

    //we only allow upto 5 voters at a time
    if(clientCount < 5)
    {
      if(pthread_create(&threads[clientCount], NULL, server_thread, &client_sock) != 0 )
      {
        printf("Failed to create thread\n");
      }

      memset(server_message,'\0',sizeof(server_message));
      memset(client_details,'\0',sizeof(client_details));
      memset(client_symbol,'\0',sizeof(client_symbol));

      //pthread_join(threads[clientCount], NULL);
    }

    else
    {
      //Receive the message from the client

      if (recv(client_sock, client_details, sizeof(client_details),0) < 0)
      {
        printf("Receive Failed. Error!!!!!\n");
        return -7;
      }

      printf("Client Message: %s\n",client_details);

      //Send the message back to client

      strcpy(server_message, "Server Full!");

      if (send(client_sock, server_message, strlen(server_message),0)<0)
      {
        printf("Send Failed. Error!!!!!\n");
        return -8;
      }

      //Disconnecting client from server
      close(client_sock);
    }
  }

  //Closing the server socket
  close(socket_desc);
  return 0;
}
