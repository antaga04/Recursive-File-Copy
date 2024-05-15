#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <syslog.h>
#include <limits.h>
#include <utime.h>

#define BUFFER_SIZE 1024

int preserve_attributes = 0;

void handle_sigint(int sig)
{
  syslog(LOG_INFO, "Copying interrupted. Deleting incomplete destination file...");
  closelog();
  exit(EXIT_FAILURE);
}

void copy_file(const char *source, const char *destination)
{
  int source_fd, dest_fd;
  ssize_t bytes_read, bytes_written;
  char buffer[BUFFER_SIZE];

  source_fd = open(source, O_RDONLY);
  if (source_fd == -1)
  {
    perror("Error opening source file");
    exit(EXIT_FAILURE);
  }

  dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (dest_fd == -1)
  {
    perror("Error opening destination file");
    exit(EXIT_FAILURE);
  }

  while ((bytes_read = read(source_fd, buffer, BUFFER_SIZE)) > 0)
  {
    bytes_written = write(dest_fd, buffer, bytes_read);
    if (bytes_written != bytes_read)
    {
      perror("Error writing to destination file");
      exit(EXIT_FAILURE);
    }
  }

  if (bytes_read == -1)
  {
    perror("Error reading from source file");
    exit(EXIT_FAILURE);
  }

  if (preserve_attributes)
  {
    struct stat st;
    if (stat(source, &st) == -1)
    {
      perror("Error getting file attributes");
      exit(EXIT_FAILURE);
    }
    if (fchmod(dest_fd, st.st_mode) == -1)
    {
      perror("Error setting file mode");
      exit(EXIT_FAILURE);
    }
    struct utimbuf new_times;
    new_times.actime = st.st_atime;
    new_times.modtime = st.st_mtime;

    if (utime(destination, &new_times) == -1)
    {
      perror("Error setting file access times");
      exit(EXIT_FAILURE);
    }
  }

  if (close(source_fd) == -1 || close(dest_fd) == -1)
  {
    perror("Error closing files");
    exit(EXIT_FAILURE);
  }
}

void copy_directory(const char *source, const char *destination)
{
  DIR *dir;
  struct dirent *entry;
  struct stat st;
  char source_path[PATH_MAX], dest_path[PATH_MAX];

  dir = opendir(source);
  if (dir == NULL)
  {
    perror("Error opening source directory");
    exit(EXIT_FAILURE);
  }

  mkdir(destination, 0777);

  while ((entry = readdir(dir)) != NULL)
  {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    {
      continue;
    }

    snprintf(source_path, PATH_MAX, "%s/%s", source, entry->d_name);
    snprintf(dest_path, PATH_MAX, "%s/%s", destination, entry->d_name);

    if (lstat(source_path, &st) == -1)
    {
      perror("Error getting file status");
      exit(EXIT_FAILURE);
    }

    if (S_ISDIR(st.st_mode))
    {
      copy_directory(source_path, dest_path);
    }
    else
    {
      copy_file(source_path, dest_path);
    }
  }

  if (closedir(dir) == -1)
  {
    perror("Error closing source directory");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    fprintf(stderr, "Usage: %s source_directory destination_directory [-a]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (argc == 4 && strcmp(argv[3], "-a") == 0)
  {
    preserve_attributes = 1;
  }

  if (signal(SIGINT, handle_sigint) == SIG_ERR)
  {
    perror("Error setting signal handler");
    exit(EXIT_FAILURE);
  }

  openlog("RecursiveFileCopy", LOG_PID | LOG_CONS, LOG_USER);

  const char *source = argv[1];
  const char *destination = argv[2];

  if (mkdir(destination, 0777) == -1 && errno != EEXIST)
  {
    perror("Error creating destination directory");
    exit(EXIT_FAILURE);
  }

  copy_directory(source, destination);

  syslog(LOG_INFO, "Copying completed successfully");
  closelog();
  return 0;
}
