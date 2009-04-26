#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ogcsys.h>
#include <network.h>

/* Constants */
#define BLOCK_SIZE		100

#define NETWORK_HOSTNAME	"nus.cdn.shop.wii.com"
#define NETWORK_PATH		"/ccs/download/"
#define NETWORK_PORT		80

/* Network variables */
static char hostip[16];
static s32  sockfd = -1;


char *Network_GetIP(void)
{
	/* Return IP string */
	return hostip;
}


s32 Network_Init(void)
{
	s32 ret;

	/* Initialize network */
	ret = if_config(hostip, NULL, NULL, true);
	if (ret < 0)
		return ret;

	return 0;
}

s32 Network_Connect(void)
{
	struct hostent *he;
	struct sockaddr_in sa;

	s32 ret;

	/* Close socket if it is already open */
	if (sockfd >= 0)
		net_close(sockfd);

	/* Create socket */
	sockfd = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (sockfd < 0)
		return sockfd;

	/* Get host by name */
	he = net_gethostbyname(NETWORK_HOSTNAME);
	if (!he)
		return -1;

	/* Setup socket */
	memcpy(&sa.sin_addr, he->h_addr_list[0], he->h_length);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(NETWORK_PORT);

	ret = net_connect(sockfd, (struct sockaddr *)&sa, sizeof(sa));
	if (ret < 0)
		return ret;

	return 0;
}

s32 Network_Request(const char *filepath, u32 *len)
{
	char buf[1024], request[256];
	char *ptr = NULL;

	u32 cnt, length;
	s32 ret;

	/* Generate HTTP request */
	sprintf(request, "GET " NETWORK_PATH "%s HTTP/1.1\r\nHost: " NETWORK_HOSTNAME "\r\nConnection: close\r\n\r\n", filepath);

	/* Connect to server */
	ret = Network_Connect();
	if (ret < 0)
		return ret;

	/* Send request */
	ret = net_send(sockfd, request, strlen(request), 0);
	if (ret < 0)
		return ret;

	/* Clear buffer */
	memset(buf, 0, sizeof(buf));

	/* Read HTTP header */
	for (cnt = 0; !strstr(buf, "\r\n\r\n"); cnt++)
		if (net_recv(sockfd, buf + cnt, 1, 0) <= 0)
			return -1;

	/* HTTP request OK? */
	if (!strstr(buf, "HTTP/1.1 200 OK"))
		return -1;

	/* Retrieve content size */
	ptr = strstr(buf, "Content-Length:");
	if (!ptr)
		return -1;

	sscanf(ptr, "Content-Length: %u", &length);

	/* Set length */
	*len = length;

	return 0;
}

s32 Network_Read(void *buf, u32 len)
{
	s32 read = 0, ret;

	/* Data to be read */
	for (read = 0; read < len; read += ret) {
		u32 size;

		/* Size to read */
		size = len - read;
		if (size > BLOCK_SIZE)
			size = BLOCK_SIZE;

		/* Read network data */
		ret = net_read(sockfd, buf + read, size);
		if (ret < 0)
			return ret;

		/* Read finished */
		if (!ret)
			break;
	}

	return read;
}

s32 Network_Write(void *buf, u32 len)
{
	s32 ret, written = 0;

	/* Data to be written */
	for (written = 0; written < len; written += ret) {
		u32 size;

		/* Size to read */
		size = len - written;
		if (size > BLOCK_SIZE)
			size = BLOCK_SIZE;

		/* Write network data */
		ret = net_write(sockfd, buf + written, size);
		if (ret < 0)
			return ret;

		/* Write finished */
		if (!ret)
			break;
	}

	return written;
}
