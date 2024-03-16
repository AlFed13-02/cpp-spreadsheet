# cpp-spreadsheet
Электронная таблица

# Описание
Проект представляет собой реализацию электронной таблицы. В ячейках таблицы могут быть текст или формулы. Формулы, могут содержать индексы ячеек. Реализована обработка следующих ошибок ввода формул: деление на 0; ячейку, индекс которой указан в формуле, нельзя интерпретировать как число; индекс ячейки выходит за границы таблицы; циклическая зависимость ячеек в формуле. Таблица эффективна по памяти, не вызывает утечек при удалении ячеек или таблицы целиком, предоставляет доступ к своим ячейкам по индексу за О(1); Для парсинга формул использовался функционал ANTLR.

# Системные требования
1. С++ 17
2. GCC 12.2.1
3. ANTLR 4.11
4. CMake 3.8
