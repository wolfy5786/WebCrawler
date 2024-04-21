#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

// Okay
// Define a structure for queue elements.
typedef struct URLQueueNode {
    char *url;
    struct URLQueueNode *next;
    struct URLQueueNode *parent;
    int depth;
} URLQueueNode;

// Define a structure for a thread-safe queue.
typedef struct {
    URLQueueNode *head, *tail;
    pthread_mutex_t lock;
} URLQueue;

typedef struct
{
  char *string;
  size_t size;
} Response;



size_t write_chunk(void *data, size_t size, size_t nmemb, void *userdata);
void initQueue(URLQueue *queue);
char *dequeue(URLQueue *queue);
void enqueue(URLQueue *queue, const char *url);

// Placeholder for the function to fetch and process a URL.
void *fetch_url(void *url);


size_t write_chunk(void *data, size_t size, size_t nmemb, void *userdata)
{
  size_t real_size = size * nmemb; 
  Response *response = (Response *) userdata;  
  char *ptr = realloc(response->string, response->size + real_size + 1);

  if (ptr == NULL)
  {
    return 0;
  }
  response->string = ptr;
  memcpy(&(response->string[response->size]), data, real_size);
  response->size += real_size;
  response->string[response->size] = '\0';
  return real_size;
}

void extract_url(char *html){
  
  char *sub;
  int i;
  int j;
  sub = strstr(html,"href=\"http");
  if(sub == NULL){
    return;
      }
  else{
    i=0;
    while(i>=0)
    {
      if(sub[i+6] == '"')
      {
        break;
      }
      i++;
    }
    char* furl = malloc(sizeof(char)*(i+1));
    j=0;
    while (j<i)
    {
      furl[j]=sub[j+6];
      j++;
    }
    furl[i]='\0';
    printf("String: %s\n",furl);
    free(furl);
    html = html+(sizeof(char)*i);
    extract_url(html);
  }

}

void *fetchurl(char *url)
{
   CURL* curl;
    CURLcode result;
    curl = curl_easy_init();
    Response response;
    response.string = malloc(1);
    response.size = 0;
    

    if(curl==NULL)
      {
	printf("REQUEST FAILED\n");
	free(response.string);
	curl_easy_cleanup(curl);
	return NULL;
      }

    curl_easy_setopt(curl, CURLOPT_URL,url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_chunk);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &response);
    result =curl_easy_perform(curl);

    if(result != CURLE_OK)
      {
	printf("Error %s\n",curl_easy_strerror(result));
	free(response.string);
	curl_easy_cleanup(curl);
	return NULL;
      }


    // printf("%s\n", response.string);

    curl_easy_cleanup(curl);
    extract_url(response.string);


    free(response.string);
    return NULL;
}
int hashing (char* url)
{
  return strlen(url)%100;
}


int main(int argc, char ** argv)
{
    if(argc <2)
    {
        printf("not enough args\n");
        return EXIT_SUCCESS;
    }
    
    char* url = argv[1];
    printf("%s\n",url);

    fetchurl(argv[1]);

    
    return EXIT_SUCCESS;
}
