#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void MultiplyMatrix(int aMatrix[][2], int bMatrix[][3], int* product, int startRow, int endRow) {
    for (int row = startRow; row < endRow; row++) {
        for (int col = 0; col < 3; col++) {
            int sum = 0;
            for (int inner = 0; inner < 2; inner++) {
                sum += aMatrix[row][inner] * bMatrix[inner][col];
            }
            product[row * 3 + col] = sum;
        }
    }
}

int main() {
    int aMatrix[3][2] = {{1, 4}, {2, 5}, {3, 6}};
    int bMatrix[2][3] = {{7, 8, 9}, {10, 11, 12}};
    int* product;

    // Создаем разделяемую память
    int shmid = shmget(IPC_PRIVATE, sizeof(int) * 3 * 3, IPC_CREAT | 0666);
    if (shmid == -1) {
        std::cerr << "Ошибка при создании разделяемой памяти!" << std::endl;
        return 1;
    }

    // Присоединяем разделяемую память
    product = (int*)shmat(shmid, NULL, 0);

    int numProcesses = 3; // Количество строк в результирующей матрице

    for (int i = 0; i < numProcesses; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            // Этот код выполняется в дочернем процессе
            int startRow = i;
            int endRow = i + 1;
            MultiplyMatrix(aMatrix, bMatrix, product, startRow, endRow);
            exit(0);
        } else if (pid < 0) {
            std::cerr << "Ошибка при создании процесса!" << std::endl;
            return 1;
        }
    }

    // Ожидание завершения всех дочерних процессов
    for (int i = 0; i < numProcesses; ++i) {
        wait(NULL);
    }

    // Вывод результата
    std::cout << "Результат перемножения матриц:" << std::endl;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            std::cout << product[row * 3 + col] << " ";
        }
        std::cout << std::endl;
    }

    // Отсоединяем разделяемую память
    shmdt(product);

    // Удаляем разделяемую память
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
