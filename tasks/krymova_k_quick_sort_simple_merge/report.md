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
```cpp
void QuickSortIterative(std::vector<int> &arr) {
    if (arr.size() <= 1) return;

    std::vector<StackItem> stack{{0, static_cast<int>(arr.size()) - 1}};

    while (!stack.empty()) {
        const auto [left, right] = stack.back();
        stack.pop_back();

        if (ShouldContinue(left, right)) continue;

        const int mid = left + ((right - left) / 2);
        
        SortThreeElements(arr, left, mid, right);
        std::swap(arr[mid], arr[right]);
        const int pivot = arr[right];
        
        const int partition = Partition(arr, left, right, pivot);
        AddToStack(stack, left, right, partition);
    }
}
```


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
Базовое время выполнения последовательной (SEQ) версии: **0.013621 секунд**.

#### Результаты параллельного выполнения:

| Processes | MPI Time (s) | Speedup vs SEQ | Данных на процесс |
|-----------|--------------|----------------|-------------------|
| 1         | 0.013698     | 0.99x          | 100,000           |
| 2         | 0.013465     | 1.01x          | 50,000            |
| 4         | 0.004998     | 2.72x          | 25,000            | 
| 6         | 0.022307     | 0.61x          | 16,666            |
| 8         | 0.008727     | 1.56x          | 12,500            |

#### Анализ результатов:

**Основные наблюдения:**

1. **Минимальные накладные расходы MPI**: 
   - 1 процесс: разница всего 0.57% с SEQ версией
   - 2 процесса: практически идентичная производительность
2. **Оптимальная конфигурация**: Максимальное ускорение 2.72x достигается при 4 процессах
3. **Проблема с 6 процессами**: Значительное замедление (на 63.7%) из-за:
   - Несбалансированного распределения: 100,000 / 6 = 16,666 (остаток 4)
   - Увеличения коммуникационных затрат
   - Некратного количества процессов размеру задачи

**Положительные аспекты:**
1. **Эффективность на 4 и 8 процессах**: Значительное ускорение (2.72x и 1.56x)
2. **Низкие накладные расходы**: MPI версия практически не уступает SEQ на 1-2 процессах

**Ограничения масштабирования:**
1. **Размер данных**: 100,000 элементов может быть недостаточно для эффективного использования 6-8 процессов
2. **Коммуникационные затраты**: Слияние отсортированных частей требует значительного обмена данными
3. **Балансировка нагрузки**: Неравномерное распределение при некруглом делении


## 7. Выводы

1. **Реалистичное масштабирование MPI**: Наблюдается ускорение с пиком 2.72x при 4 процессах

3. **Минимальные накладные расходы**: MPI реализация демонстрирует высокую эффективность - разница с SEQ версией менее 1% на 1-2 процессах

4. **Оптимальная конфигурация**: Для массива размером 100,000 элементов оптимальным является использование 4 процессов

5. **Важность балансировки**: Проблема с 6 процессами (замедление на 63.7%) показывает критическую важность сбалансированного распределения данных

6. **Практические рекомендации**:
   - Для задач подобного размера рекомендуется использовать 2-4 процесса
   - При увеличении количества процессов необходимо увеличивать размер данных
   - Важно учитывать кратность размера данных количеству процессов

## 8. Литература
1. Стандарт MPI.
2. Лекции и практики по параллельному программированию.

## 9. Приложение
```cpp
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
```