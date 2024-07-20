# Имя компилятора
CXX = g++

# Флаги компилятора
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

# Целевой файл
TARGET = php

# Исходные файлы
SRCS = php.cpp 

# Заголовочные файлы
HEADERS =

# Объектные файлы
OBJS = $(SRCS:.cpp=.o)

# Правило по умолчанию
all: $(TARGET)

# Правило для создания целевого файла
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	rm -f $(OBJS)

# Правило для создания объектных файлов
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Правило для очистки всех файлов
clean:
	rm -f $(TARGET) $(OBJS)	

# Устанавливаем файл, который следует обновить, если изменится какой-либо из его зависимых файлов
.PHONY: all clean 

