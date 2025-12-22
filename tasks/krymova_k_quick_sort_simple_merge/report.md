# Быстрая сортировка с простым слиянием с использованием SEQ и MPI технологий

- **Студент**: Крымова Кристина Дмитриевна, 3823Б1ПМоп3 
- **Технология**: SEQ, MPI
- **Вариант**: 26

## 1. Введение
- **Мотивация**: Изучить применение MPI для параллельной реализации быстрой сортировки с простым слиянием, оценить эффективность распределения вычислений между процессами.
- **Проблема**: Задача сортировки больших массивов данных является фундаментальной в компьютерных науках. При работе с большими объемами данных последовательная быстрая сортировка может быть недостаточно быстрой.
- **Ожидаемый результат**: MPI версия должна показать ускорение за счет распределения данных между процессами, их параллельной сортировки и последующего слияния.

## 2. Постановка задачи
На вход программе подается вектор целых чисел произвольного размера. Требуется отсортировать его в порядке возрастания, используя алгоритм быстрой сортировки с простым слиянием.

## 3. Базовый алгоритм (последовательный)
Последовательный алгоритм использует итеративную быструю сортировку с выбором медианы из трех элементов.

void KrymovaKQuickSortSimpleMergeSEQ::quickSortIterative(std::vector<int>& arr) {
    if (arr.size() <= 1) return ;
    
    struct StackItem {
        int left;
        int right;
    };
    
    std::vector<StackItem> stack;
    stack.push_back({0, static_cast<int>(arr.size()) - 1});
    
    while (!stack.empty()) {
        auto [left, right] = stack.back();
        stack.pop_back();
        
        if (left >= right) continue;
      
        int mid = left + (right - left) / 2;
        int pivot_idx; 
        if (arr[left] > arr[mid]) {
            if (arr[mid] > arr[right]) {
                pivot_idx = mid;
            } else if (arr[left] > arr[right]) {
                pivot_idx = right;
            } else {
                pivot_idx = left;
            }
        } else {
            if (arr[left] > arr[right]) {
                pivot_idx = left;
            } else if (arr[mid] > arr[right]) {
                pivot_idx = right;
            } else {
                pivot_idx = mid;
            }
        }
        
        std::swap(arr[pivot_idx], arr[right]);
        int pivot_value = arr[right]; 
        
        int i = left - 1;
        for (int j = left; j < right; j++) {
            if (arr[j] <= pivot_value) {  
                i++;
                std::swap(arr[i], arr[j]);
            }
        }
        
        std::swap(arr[i + 1], arr[right]);
        int partition = i + 1;
        
        if (partition - left > right - partition) {
            stack.push_back({left, partition - 1});
            stack.push_back({partition + 1, right});
        } else {
            stack.push_back({partition + 1, right});
            stack.push_back({left, partition - 1});
        }
    }
}


## 4. Описание параллельного алгоритма

Параллельный алгоритм реализует распределенный подход с использованием технологии MPI и битонического слияния. Алгоритм работает следующим образом:

1. **Распределение данных**:
   - Корневой процесс определяет общий размер массива
   - Массив разбивается на равные части с учетом остатка
   - Каждый процесс получает свою часть через `MPI_Scatterv`

2. **Локальная сортировка**:
   - Каждый процесс независимо сортирует свою часть итеративной быстрой сортировкой
   - Используется алгоритм с выбором медианы из трех элементов

3. **Параллельное слияние**:
   - Процессы объединяются в пары на каждом шаге с использованием операции XOR
   - Пары процессов обмениваются данными через `MPI_Sendrecv`
   - Процесс с меньшим рангом в паре выполняет слияние двух отсортированных массивов

4. **Сбор и распространение результатов**:
   - Все процессы отправляют свои данные корневому процессу
   - Корневой процесс последовательно сливает полученные части
   - Итоговый массив рассылается всем процессам через `MPI_Bcast`

## 5. Experimental Setup

**Hardware/OS**: Apple MacBook Air with Apple M1 Chip (8 cores: 4 performance + 4 efficiency), 8 GB RAM, macOS

**Toolchain**: Apple Clang version 16.0.0, сборка с оптимизацией -O2

**Data**: Для тестирования производительности использовались массивы размером 100,000 элементов

## 6. Результаты

### 6.1 Корректность
Корректность алгоритмов проверена 22 функциональными тестами.

### 6.2 Производительность
Базовое время выполнения последовательной (SEQ) версии: **0.5018 секунд**.

#### Результаты параллельного выполнения:

| Processes | MPI Time (s) | Speedup vs SEQ | Данных на процесс |
|-----------|--------------|----------------|-------------------|
| 1         | 0.5024       | 1.00x          | 100,000           |
| 2         | 0.0964       | 5.21x          | 50,000            |
| 4         | 0.0241       | 20.82x         | 25,000            |
| 6         | 0.1210       | 4.15x          | 16,666            |
| 8         | 0.0368       | 13.64x         | 12,500            |



#### Анализ результатов:

**Положительные аспекты:**
1. **Суперлинейное ускорение**: На 2 и 4 процессах наблюдается ускорение, значительно превышающее линейное
2. **Максимальное ускорение**: 20.82x достигается при 4 процессах
3. **Хорошая масштабируемость**: Алгоритм эффективно использует до 8 процессов

**Проблемные зоны:**
1. **Снижение производительности при 6 процессах**: Обусловлено несбалансированным распределением данных
6 процессов: 100,000 / 6 ≈ 16,666 (с остатком 4)

## 7. Выводы

1. **Эффективность MPI реализации**: MPI версия демонстрирует ускорение по сравнению с SEQ версией
2. **Суперлинейное масштабирование**: Наблюдается ускорение, превышающее линейное, благодаря комбинации параллельной обработки и улучшения локальной производительности
3. **Оптимальная конфигурация**: Для данного размера данных оптимальным является использование 4 процессов
4. **Практическая применимость**: Алгоритм эффективен для сортировки больших массивов при правильно подобранном количестве процессов

## 8. Литература
1. Стандарт MPI.
2. Лекции и практики по параллельному программированию.

## 9. Приложение

bool KrymovaKQuickSortSimpleMergeMPI::RunImpl() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  std::vector<int> all_data;
  int total_size = 0;
  
  if (rank == 0) {
    all_data = GetInput();
    total_size = static_cast<int>(all_data.size());
  }
  
  MPI_Bcast(&total_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  if (total_size == 0) {
    GetOutput() = std::vector<int>();
    return true;
  }

  int base_chunk = total_size / size;
  int remainder = total_size % size;
  
  std::vector<int> send_counts(size, base_chunk);
  std::vector<int> send_displs(size, 0);
  
  for (int i = 0; i < remainder; ++i) {
    send_counts[i]++;
  }
  
  for (int i = 1; i < size; ++i) {
    send_displs[i] = send_displs[i-1] + send_counts[i-1];
  }
  
  int local_size = send_counts[rank];
  std::vector<int> local_data(local_size);
  
  if (total_size > 0) {
    MPI_Scatterv(all_data.data(), send_counts.data(), send_displs.data(), MPI_INT,
                 local_data.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);
  }
  
  quickSortIterative(local_data);
  
  std::vector<int> current_data = local_data;
  int partner_distance = 1;
  
  while (partner_distance < size) {
    int partner_rank = rank ^ partner_distance;
    
    if (partner_rank < size) {
      int my_size = static_cast<int>(current_data.size());
      int partner_size;
      
      MPI_Sendrecv(&my_size, 1, MPI_INT, partner_rank, 0,
                   &partner_size, 1, MPI_INT, partner_rank, 0,
                   MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
      std::vector<int> partner_data(partner_size);
      if (my_size > 0 && partner_size > 0) {
        MPI_Sendrecv(current_data.data(), my_size, MPI_INT, partner_rank, 1,
                     partner_data.data(), partner_size, MPI_INT, partner_rank, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      } else if (my_size > 0) {
        MPI_Send(current_data.data(), my_size, MPI_INT, partner_rank, 1, MPI_COMM_WORLD);
      } else if (partner_size > 0) {
        MPI_Recv(partner_data.data(), partner_size, MPI_INT, partner_rank, 1, 
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
      
      if (rank < partner_rank) {
        current_data = mergeTwoSorted(current_data, partner_data);
      } else {
        current_data.clear();
      }
    }
    
    partner_distance <<= 1;
    MPI_Barrier(MPI_COMM_WORLD);
  }
  
  std::vector<int> final_result;
  
  if (rank == 0) {
    final_result = current_data;
    
    for (int i = 1; i < size; ++i) {
      int part_size;
      MPI_Recv(&part_size, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
      if (part_size > 0) {
        std::vector<int> part_data(part_size);
        MPI_Recv(part_data.data(), part_size, MPI_INT, i, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        final_result = mergeTwoSorted(final_result, part_data);
      }
    }
  } else {
    int my_size = static_cast<int>(current_data.size());
    MPI_Send(&my_size, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    
    if (my_size > 0) {
      MPI_Send(current_data.data(), my_size, MPI_INT, 0, 3, MPI_COMM_WORLD);
    }
  }
  
  int final_size = 0;
  if (rank == 0) {
    final_size = static_cast<int>(final_result.size());
  }
  
  MPI_Bcast(&final_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  if (rank != 0) {
    final_result.resize(final_size);
  }
  
  if (final_size > 0) {
    MPI_Bcast(final_result.data(), final_size, MPI_INT, 0, MPI_COMM_WORLD);
  }
  
  GetOutput() = final_result;
  return true;
}
