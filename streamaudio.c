#include <alsa/asoundlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>

#define CHUNK 256

static snd_pcm_t *sound_dev;
static char      *buf;

void sig_handler(int signum);

int main(int argc, char **argv)
{
    /* Audio variables */
    snd_pcm_hw_params_t *sound_dev_params;
    snd_pcm_uframes_t frames = CHUNK, fr_tmp;
    unsigned int rate = 48000, channels = 1, du_tmp;
    int err;

    /* Socket variables */
    int socket_fd;
    struct sockaddr_in server_addr, client_addr;

    /* Set signal handler */
    signal(SIGINT, sig_handler);

    /* Initiate socket */
    if((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    {
        printf("Can't create socket\n");
        return -1;
    }
    
    /* Set port and IP */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(11500);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(12500);
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.255");

    /* Bind to the set port and IP */
    if(bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("can't bind address to the socket\n");
        return -1;
    }

    const int ben=1;
    if(setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, &ben, sizeof(ben)) < 0)
    {
        printf("Can't set broadcast for socket\n");
        return -1;
    }

    /* Initiate audio */
	if (argc < 4) {
		printf("Usage: %s <sample_rate> <channels> <device>\n",
								argv[0]);
		return -1;
	}
 
	rate 	 = atoi(argv[1]);
	channels = atoi(argv[2]);
 
	/* Open the PCM device in playback mode */
    if(err = snd_pcm_open(&sound_dev, argv[3], SND_PCM_STREAM_CAPTURE, 0) < 0)
    {
        printf("Can't open device: %s\n", snd_strerror(err));
        return -1;
    }

    /* set parameters */
    snd_pcm_hw_params_alloca(&sound_dev_params);
    snd_pcm_hw_params_any(sound_dev, sound_dev_params);

    if(err = snd_pcm_hw_params_set_access(sound_dev, sound_dev_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
    {
        printf("Can't set access: %s\n", snd_strerror(err));
        return -1;
    }

    if(err = snd_pcm_hw_params_set_format(sound_dev, sound_dev_params, SND_PCM_FORMAT_S16_LE) < 0)
    {
        printf("Can't set format: %s\n", snd_strerror(err));
        return -1;
    }

    if(err = snd_pcm_hw_params_set_channels(sound_dev, sound_dev_params, channels) < 0)
    {
        printf("Can't set channels: %s\n", snd_strerror(err));
        return -1;
    }

    if(err = snd_pcm_hw_params_set_rate_near(sound_dev, sound_dev_params, &rate, 0) < 0)
    {
        printf("Can't set rate: %s\n", snd_strerror(err));
        return -1;
    }

    if(err = snd_pcm_hw_params_set_period_size(sound_dev, sound_dev_params, frames, 0) < 0)
    {
        printf("Can't set frames: %s\n", snd_strerror(err));
        return -1;
    }

    if(err = snd_pcm_hw_params(sound_dev, sound_dev_params) < 0)
    {
        printf("Can't set params: %s\n", snd_strerror(err));
        return -1;
    }

    snd_pcm_prepare(sound_dev);

    snd_pcm_hw_params_get_period_time(sound_dev_params, &du_tmp, 0);
    printf("du_tmp    = %d\n", du_tmp);
    snd_pcm_hw_params_get_period_size(sound_dev_params, &fr_tmp, 0);
    printf("frame_tmp = %d\n", fr_tmp);

	int   buf_size = fr_tmp * channels * 2 /* 2 -> sample size */;
	buf = (char *) malloc(buf_size);
    printf("buf_size  = %d\n", buf_size);


	while (1) 
    {
        if(err = snd_pcm_readi(sound_dev, buf, fr_tmp) < fr_tmp)
        {
            printf("Reading wasn't completed: %s\n", snd_strerror(err));
        }
        if (sendto(socket_fd, buf, buf_size, 0,\
        (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0)
        {
            printf("Can't send data to client\n");
            return -1;
        }
	}
    return 0;
}

void sig_handler(int signum)
{
    printf("SIGINT handled\n");
    snd_pcm_close(sound_dev);
    free(buf);
    exit(0);
}