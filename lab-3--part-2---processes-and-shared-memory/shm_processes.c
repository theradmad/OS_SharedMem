#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/ipc.h>
#include  <sys/shm.h>
#include  <unistd.h>
#include  <sys/wait.h>
#include  <time.h>

// Shared memory structure
struct SharedData {
    int BankAccount;
    int Turn;
};

void DearOldDad(struct SharedData *data) {
    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6); // Sleep for a random time (0-5 seconds)

        // Wait for it to be Dad's turn
        while (data->Turn != 0)
            ; // Do nothing

        int account = data->BankAccount;

        if (account <= 100) {
            int balance = rand() % 101; // Random balance amount to give the student
            if (balance % 2 == 0) {
                account += balance; // Deposit the amount into the account
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        data->BankAccount = account; // Update shared account balance
        data->Turn = 1; // Set the turn to the child
    }
}

void PoorStudent(struct SharedData *data) {
    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6); // Sleep for a random time (0-5 seconds)

        // Wait for it to be Student's turn
        while (data->Turn != 1)
            ; // Do nothing

        int account = data->BankAccount;
        int balance = rand() % 51; // Random balance amount the student needs

        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) {
            account -= balance; // Withdraw the amount from the account
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        data->BankAccount = account; // Update shared account balance
        data->Turn = 0; // Set the turn to Dad
    }
}

int main() {
    int ShmID;
    struct SharedData *ShmPTR;
    pid_t pid;
    int status;

    srand(time(0)); // Initialize random seed

    ShmID = shmget(IPC_PRIVATE, sizeof(struct SharedData), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error ***\n");
        exit(1);
    }

    ShmPTR = (struct SharedData *)shmat(ShmID, NULL, 0);
    ShmPTR->BankAccount = 0;
    ShmPTR->Turn = 0;

    pid = fork();
    if (pid < 0) {
        printf("*** fork error ***\n");
        exit(1);
    } else if (pid == 0) {
        DearOldDad(ShmPTR);
    } else {
        PoorStudent(ShmPTR);
    }

    wait(&status);

    shmdt((void *)ShmPTR);
    shmctl(ShmID, IPC_RMID, NULL);

    return 0;
}
