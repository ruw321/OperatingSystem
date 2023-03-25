#include "pennfat.h"

int main(int argc, char *argv[]) {
  // Check if the number of arguments is correct
  if (argc != 1) {
    printf("No arguments are needed for this program.\n");
    return -1;
  }
  
  while (true) {
    // Print the prompt
    printf("pennFAT> ");

    // Read the command
    char cmdLine[MAX_CMD_LEN];
    char* args[MAX_ARGS];
    int numBytes = read(STDIN_FILENO, cmdLine, MAX_CMD_LEN);

    // Check if the command is valid
    if (numBytes == -1) {
      printf("Error reading command.\n");
      continue;
    } else if (numBytes == 0) {
      printf("Exiting pennFAT.\n");
      break;
    }

    // Parse the command
    int numArgs = 0;
    char *token = strtok(cmdLine, "\n");
    token = strtok(token, " ");
    
    while (token != NULL) {
      token = strtok(NULL, " ");
      args[numArgs] = token;
      numArgs++;
    }

    if (numArgs == 0) {
      continue;
    }

    // Execute the command
    char *op = args[0];
    if (strcmp(op, MAKE_FILESYSTEM) == 0) {
      if (numArgs != 4) {
        printf("Usage: mkfs FS_NAME BLOCKS_IN_FAT BLOCK_SIZE_CONFIG\n");
        continue;
      }
      makeFileSystem(args[1], atoi(args[2]), atoi(args[3]));
    } else if (strcmp(op, MOUNT_FILESYSTEM) == 0) {
      if (numArgs != 2) {
        printf("Usage: mount FS_NAME\n");
        continue;
      }
      mountFileSystem(args[1]);
    } else if (strcmp(op, UNMOUNT_FILESYSTEM) == 0) {
      if (numArgs != 1) {
        printf("Usage: umount\n");
        continue;
      }
      unmountFileSystem();
    } else if (strcmp(op, TOUCH_FILE) == 0) {
      if (numArgs < 2) {
        printf("Usage: touch FILE ...\n");
        continue;
      }
      touchFile(args, numArgs-1);
    } else if (strcmp(op, MOVE_FILE) == 0) {
      if (numArgs != 3) {
        printf("Usage: mv SOURCE DEST\n");
        continue;
      }
      moveFile(args[1], args[2]);
    } else if (strcmp(op, REMOVE_FILE) == 0) {
      if (numArgs != 2) {
        printf("Usage: rm FILE ...\n");
        continue;
      }
      removeFile(args, numArgs-1);
    } else if (strcmp(op, CAT_FILE) == 0) {
      if (numArgs < 2) {
        printf("Usage: cat -w/-a OUTPUT_FILE || cat FILE ... [ -w/-a OUTPUT_FILE ]\n");
        continue;
      }
      if (strcmp(args[1], "-w") == 0)
        catFile(NULL, args[2], true, false, 0);
      else if (strcmp(args[1], "-a") == 0)
        catFile(NULL, args[2], false, true, 0);
      else {
        int overwrite = false;
        int append = false;
        for (int i = 1; i < numArgs; i++) {
          if (strcmp(args[i], "-w") == 0) {
            overwrite = true;
            break;
          }
          if (strcmp(args[i], "-a") == 0) {
            append = true;
            break;
          }
        }
        if (!overwrite && !append)
          catFile(args[1], NULL, false, false, numArgs-1);
        else if (overwrite)
          catFile(args[1], args[numArgs-1], true, false, numArgs-3);
        else if (append)
          catFile(args[1], args[numArgs-1], false, true, numArgs-3);
      }
    } else if (strcmp(op, COPY_FILE) == 0) {
      if (numArgs < 3) {
        printf("Usage: cp [ -h ] SOURCE DEST || cp SOURCE -h DEST\n");
        continue;
      }
      if (strcmp(args[1], "-h") == 0)
        copyFile(args[2], args[3], true, false);
      else if (strcmp(args[2], "-h") == 0)
        copyFile(args[1], args[3], false, true);
      else
        copyFile(args[1], args[2], false, false);
    } else if (strcmp(op, LIST_FILES) == 0) {
      if (numArgs != 1) {
        printf("Usage: ls\n");
        continue;
      }
      listFiles();
    } else if (strcmp(op, CHANGE_PERMISSIONS) == 0) {
      if (numArgs != 3) {
        printf("Usage: chmod MODE FILE\n");
        continue;
      }
      changePermissions(args[1], args[2]);
    }
  }
}