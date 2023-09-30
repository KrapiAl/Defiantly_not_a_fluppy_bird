// Подключение используемых библиотек
#include <stdio.h>
#include <time.h>
#include "SFML/Graphics.hpp"

// Использование пространства имен sf по умолчанию
using namespace sf;

// Глобальные переменные, необходимы для взаимодействия с разными функциями
const int width = 1500; 
const int height = 1000;
RenderWindow window(VideoMode(width, height), "Definitely not Flappy Bird Game");


// Класс объекта трубы
class Tower
{
// Недоступное из вне класса значение
private:
    // Текстура трубы
    Texture texture;

// Переменные с публичным доступом 
public:

    // Объявление переменных класса
    float height, width; // Параметры трубы (высота и ширина)
    Sprite up_tower, down_tower; // Спрайты верхней и нижней труб
    bool moving; // Бинарная переменная передвижения (двигается данная труба или нет)
    Vector2f position; // Вектор с двумя переменнами типа float (позиция трубы)
    int delta = 300; // Расстояние между верхней и нижней трубой
    bool passed = false; // Бинарная переменная прохождения трубы (прошел эту трубу игрок или нет)

    // Объявление функций класса 

    // Установка значений по умолчанию
    void SetDefault(const char path[]) {
        // Программа принимает аргумент - строку, указывающую путь к текстуре в дирректории проекта

        float scale = 7; // Масштаб трубы (увеличение спрайта в 7 раз)

        texture.loadFromFile(path); // Загрузка текстуры из файла
        Vector2u size = texture.getSize(); // Вектор с двумя переменнами типа usigned int (размер текстуры)

        up_tower.setTexture(texture); // Установка текстуры на верхнюю трубу
        up_tower.setScale(scale, -1 * scale); // Масштабирование верхней трубы
        down_tower.setTexture(texture); // Установка текстуры на нижнюю трубу
        down_tower.setScale(scale, scale); // Масштабирование нижней трубы

        width = size.x * scale; // Ширина трубы
        height = size.y * scale; // Высота трубы

        moving = false; // По умолчанию труба не двигается
    }
};


// Структура игрока (птицы)
struct Player
{
// Переменные с публичным доступом 
public:

    // Объявление переменных класса
    float spd_x, spd_y; // Скорость игрока по двум осям
    FloatRect rect; // Положение игрока (квадрата)
    Sprite sprite; // Спрайт игрока
    float current_frame; // Текуший кадр анимации
    bool direction = true; // Направление движения

    // Объявление функций класса 

    // Установка значений по умолчанию
    Player(Texture &image) {
        // Программа принимает аргумент - указатель на текстуру
        sprite.setTexture(image); // Загрузка текстуры
        sprite.setScale(0.25, 0.25); // Масштабирование игрока (уменьшение в 4 раза)
        rect = FloatRect(10, 200, 150, 100); // Начальное положение на экране
        sprite.setTextureRect(IntRect(600, 0, 600, 400)); // Кадрирование текстуры

        spd_x = spd_y = 0; // Начальная скорость
        current_frame = 0; // Начальный кадр анимации
    }

    // Обновление позиции на экране
    void update_position(float time) {
        // Программа принимает аргумент - время в микросекундах, прошедшое с предыдущего цикла процессора
        rect.left += spd_x * time; // Смещение по горизонтали
        if ((rect.top < (height - rect.height)) || (spd_y < 0)) {
            rect.top += spd_y * time; // Смещение по вертикали
        }

        sprite.setPosition(rect.left, rect.top); // Изменение положение спрайта
        sprite.setRotation(spd_y * 8); // Поворот спрайта

        // Направление персонажа
        if (direction) {
            sprite.setTextureRect(IntRect(600 * int(current_frame), 0, 600, 400)); // Поворот направо
        } else {
            sprite.setTextureRect(IntRect(600 * int(current_frame) + 600, 0, -600, 400)); // Поворот налево
        }

        // Обновление анимации
        current_frame += 0.005 * time; // Текущий кадр анимации
        if (current_frame >= 4) { current_frame = 0; } // Сброс анимации

        // Изменение скорости персонажа
        if (spd_y < 1.3) { spd_y += 0.006 * time; } // Увеличение вертикальной скорости (т.к. игрок падает)
        if (spd_x > 0) { spd_x -= 0.0004 * time; } // Уменьшение горизонтальной скорости (чтобы персонаж останавливался)
        if (spd_x < 0) { spd_x += 0.0004 * time; } // Уменьшение горизонтальной скорости (чтобы персонаж останавливался)
    }

    // Проверка на выход за пределы жкрана по горизонтали (не используется)
    void collision() {
        if ((rect.left <= 0) || ((rect.left + rect.width) > width)) {
            spd_x = 0;
        }
    }
};


// Функция, возвращающая случайное число из заданного диапозона
int rand_value(int min, int max)
{
    // Программа принимает аргументы - нижнюю и верхнюю границу диапозона

    return (rand() % (max - min) + min);
}


// Функция обновление и усправления трубами


// Отдельная функция отрисовки движущихся труб
void tower_draw(Tower towers[], int cnt)
{
    for (int i = 0; i < cnt; i++) {
        if (towers[i].moving) {
            // Отображение (отрисовка) трубы на экране
            window.draw(towers[i].up_tower); // Отрисовка верхней трубы
            window.draw(towers[i].down_tower); // Отрисовка нижней трубы
        }
    }
}

// Изменение параметров и отрисовка труб
void control_tower(Tower towers[], int cnt, float dist, int* first_tower, float time, int* score)
{
    // Программа принимает аргументы:
        // Tower towers[] - массив объектов класса Tower (массив с трубами)
        // int cnt - количество труб 
        // float dist - установленная дистанция между трубами (расстояние, после которого следующая труба начинает двигаться)
        // int* first_tower - указатель на индекс первой трубы
        // float time - время в микросекундах, прошедшое с предыдущего цикла процессора
        // int* score - указатель на игровой счет

    static int last_tower = 0; // Последняя (правая) труба
    float spd_twr = 0.45; // Скорость перемещения труб

    // Применение параметров для каждой трубы
    for (int i = 0; i < cnt; i++) {

        // Если труба двигается
        if (towers[i].moving) {

            // Позиция трубы
            float pos_x = towers[i].position.x; // По оси X
            float pos_y = towers[i].position.y; // По оси Y

            // Изменение позиции трубы
            towers[i].up_tower.setPosition(pos_x, pos_y - towers[i].delta); // Для верхней
            towers[i].down_tower.setPosition(pos_x, pos_y); // Для нижней

            // Расчёт новой позиции
            pos_x = pos_x - spd_twr * time; // Смещение по горизонтали
            towers[i].position.x = pos_x; // Сохранение новой позции

            // Отображение (отрисовка) трубы на экране
            window.draw(towers[i].up_tower); // Отрисовка верхней трубы
            window.draw(towers[i].down_tower); // Отрисовка нижней трубы
        }
    }

    // Взаимодействие с последней трубой 
    if (width - (towers[last_tower].position.x + towers[last_tower].width) >= dist) {
        if (last_tower == cnt - 1) { last_tower = 0; } // Следующая труба (переход от последней к первой)
        else { last_tower += 1; } // Следующая труба

        // Обновление параметров для последней трубы
        towers[last_tower].position.x = width; // Перемещение трубы за пределы экрана (за правую границу)
        towers[last_tower].moving = true; // Труба двигается
        towers[last_tower].delta = 300 - 25 * (*score / 5); // Новое расстояние между верхней и нижней трубой (уменьшается со временем)
        towers[last_tower].position.y = rand_value(height - (int)towers[last_tower].height, height - 120); //Положение по вертикали
        towers[last_tower].passed = false; // Труба не пройдена
    }

    //Взаимодействие с первой (левой) трубой
    if (towers[*first_tower].position.x + towers[*first_tower].width <= 0) { // Условие, при котором труба уходит за пределы экрана
        if (*first_tower == cnt - 1) { *first_tower = 0; } // Следующая труба (переход от последней к первой)
        else { *first_tower += 1; } // Следующая труба 
    }
}

// Проверка пересечение игрока с первой (левой) трубой
bool collision_check(Player *player, Tower *tower, int *score)
{
    // Программа принимает аргументы - указатели на объект игрока, первой (левой) трубы и счета

    // Установка (инициализация) переменных
    bool crash = 0; // По умолчанию игрок не врезался в трубу
    float pos_y = player->rect.top + 10; // Позиция игрока по вертикали (со смещением)
    float pos_x = player->rect.left + 20; // Позиция игрока по горизотали (со смещением)
    float high = player->rect.height - 30; // Высота игрока (со смещением)
    float wide = player->rect.width - 40; // Ширина игрока (со смещением)

    // Проверка выхода игрока за пределы экрана по вертикали
    if (pos_y < 0) { player->rect.top = 0; }
    else if (pos_y + high > height) { player->rect.top = height - high; }

    // Проверка пересечения игрока с трубой
    if (pos_y <= tower->up_tower.getPosition().y || pos_y + high >= tower->down_tower.getPosition().y) {
        if (pos_x + wide >= tower->position.x && pos_x <= tower->position.x + tower->width) {
            crash = 1; // В случае пересечения считается, что игрок врезался в трубу
        }
    }

    // Проверка прохождения трубы
    if (pos_x > tower->position.x) {
        if (!tower->passed) {
            *score += 1; // Увеличение счета игрока, если труба не была пройдена
        }
        tower->passed = true; // Труба пройдена
    }
    return crash; // Возвращение результата проверки коллизии
}


// Основная функция (сама игра)
bool game()
{

    // Объявление используемых текстур
    Texture, bird_texture, backgound, score_texture, lose_texture, paused_texture;
    bird_texture.loadFromFile("assets/bird.png"); // Загрузка текстуры птицы
    backgound.loadFromFile("assets/background.png"); // Загрузка текстуры заднего фона
    score_texture.loadFromFile("assets/digits.png"); // Загрузка текстуры цифр счета
    lose_texture.loadFromFile("assets/game_over.png"); // Загрузка текстуры надписи
    paused_texture.loadFromFile("assets/paused.png"); // Загрузка текстуры паузы
    
    // Инициализация игрока (создание переменной класса игрок)
    Player bird(bird_texture); // Текстура передается как аргумет инициализации для установки ее на спрайт птицы
    
    // Инициализация спрайта заднего фона
    Sprite city(backgound); // Текстура заднего плана
    city.setTextureRect(IntRect(0, 12, 5400, 1000)); // Кадрирование текстуры 
    float scrolling = 0; // Переменная перемещение заднего фона

    // Инициализация двух цифр счета
    Sprite number1, number2;
    number1.setTexture(score_texture); // Установка текстуры 
    number1.setTextureRect(IntRect(0, 0, 10, 12)); // Кадрирование текстуры
    number1.setScale(7, 7); // Масштабирование цифры (увеличение в 7 раз)
    number1.setPosition(600, 50); // Установка позиции

    number2.setTexture(score_texture); // Установка текстуры
    number2.setTextureRect(IntRect(0, 0, 10, 12)); // Кадрирование текстуры
    number2.setScale(7, 7); // Масштабирование цифры (увеличение в 7 раз)
    number2.setPosition(670, 50); // Установка позиции

    // Инициализация спрайта надписи
    Sprite lose;
    lose.setTexture(lose_texture); // Установка текстуры
    lose.setScale(10, 10); // Масштабирование надписи (увеличение в 10 раз)
    lose.setPosition(300, 200); // Установка позиции

    // Инициализация спрайта паузы
    Sprite pause;
    pause.setTexture(paused_texture); // Установка текстуры
    pause.setScale(10, 10); // Масштабирование надписи (увеличение в 10 раз)
    pause.setPosition(400, 200); // Установка позиции
    bool paused = false; // Истино, когда оставлена игра на паузу

    // Инициализация переменных, связанных с трубами
    const int cnt_tower = 4; // Количество труб
    int first_tower = 0; // Индекс первой (левой) трубы
    Tower towers[cnt_tower]; // Массив с объектами (трубами)

    // Установка значение по умолчанию для элементов массива
    for (int i = 0; i < cnt_tower; i++) {
        towers[i].SetDefault("assets/temp_pipe.png"); // Функция установки значений по умолчанию

        towers[i].position.x = width; // По умолчанию труба стоит за правой границей экрана
        towers[i].position.y = rand_value(height - (int)towers[i].height, height - 120); // Случайное значение по вретикали
    }

    // Изменение значений для первой трубы
    towers[first_tower].moving = true; // Первая труба должна двигаться
    towers[first_tower].position.x = 2000; // Сдвиг трубы чуть дальше, чтобы у игрока было время 
    
    // Инициализация переменных, использующихся в игре
    Clock clock; // Создание объекта класса часов
    bool game_over = false, lose_shown = false; // Две бинарные переменные, связанные с окончанием игры
    bool space_pressed = false; // Бинарная переменная нажатия на пробел
    bool esc_pressed = false; // Бинарная переменная нажатия на Esc
    int score = 0; // Игровой счет

    // Основное тело функции (игровой цикл)
    while (window.isOpen()) {
        // Переменные времни игры
        float time = clock.getElapsedTime().asMicroseconds(); // time - время в макросекундах с предыдущего цикла процессора
        clock.restart(); // Отсчет времени от текущего цикла процессора

        time = time / 1000; // Перевод макросекунд в микросекунды

        Event event; // Объект класса события

        while (window.pollEvent(event)) { // Для всех событий
            if (event.type == Event::Closed) { window.close(); return false; } // Закрыть окно программы, если его закрыли
        }

        if (!game_over) { // Если игра не окончена
            window.clear(); // Очистить окно (удалить все изображения)
            window.draw(city); // Отрисовать задний план

            // Управление с клавиатуры
            if (Keyboard::isKeyPressed(Keyboard::Escape) && !esc_pressed) { // Если нажата кнопка "Esc"
                esc_pressed = true;
                if (paused) {
                    paused = false; // Если игра не поставлена на паузу, ставим ее на паузу
                }
                else {
                    paused = true; // Если игра поставлена на паузу, то отпускаем ее с паузы
                }
            }
            if (!Keyboard::isKeyPressed(Keyboard::Escape) && esc_pressed) { // Если Esc отжат (перестали нажимать)
                esc_pressed = false; // Esc больше не нажат
            }
            if (Keyboard::isKeyPressed(Keyboard::Left)) { // Есди нажата стрелочка влево
                bird.direction = false; // Направление движения влево
                if (bird.spd_x > -0.4) { bird.spd_x -= 0.002 * time; } // Уменьшить горизонтальную скорость
            }
            if (Keyboard::isKeyPressed(Keyboard::LShift)) { // Если нажат левый шифр
                bird.spd_y = 0; // Удержание высоты
            }
            if (Keyboard::isKeyPressed(Keyboard::Right)) { // Если нажата стрелочка вправо
                bird.direction = true; // Напраление движения вправо
                if (bird.spd_x < 0.4) { bird.spd_x += 0.002 * time; } // Увеличить горизонтальную скорость
            }
            if (Keyboard::isKeyPressed(Keyboard::Space) && !space_pressed) { // Если нажат (и не зажат) пробел
                space_pressed = true; // Пробел нажат
                bird.spd_y = -1; // Изменить вертикальную скорость
            }
            if (!Keyboard::isKeyPressed(Keyboard::Space) && space_pressed) { // Если пробел отжат (перестали нажимать)
                space_pressed = false; // Пробел больше не нажат
            }


            // Обновление параметров объектов игры (использование функций), если игра не стоит на паузе
            if (!paused) {
                bird.update_position(time); // Обновление позиции игрока
                game_over = collision_check(&bird, &towers[first_tower], &score); // Проверка пересечения игрока с левой колонной
                control_tower(towers, cnt_tower, 500, &first_tower, time, &score); // Управление (изменение положения) колонн
            }

            // Изменение спрайтов цифр счета
            number1.setTextureRect(IntRect(10 * ((score / 10) % 5), 12 * ((score / 10) / 5), 10, 12)); // Первая цифра
            number2.setTextureRect(IntRect(10 * ((score % 10) % 5), 12 * ((score % 10) / 5), 10, 12)); // Правая цифра
            
            // Изменение параметров заднего фона, если игра не поставлена на паузу
            if (!paused) {
                scrolling -= 0.1 * time; // Переменная перемещения заднего фона
                if (scrolling <= -3780) { scrolling = 0; } // Сброс переменной, если дошли до конца заднего фона
                city.setPosition(scrolling, 0); // Установка нового положения спрайта фона
            }

            // Отрисовка объектов игры
            window.draw(number1); window.draw(number2); // Отрисовка счета (двух цифр)
            window.draw(bird.sprite); // Отрисовка игрока
            if (paused) {
                tower_draw(towers, cnt_tower); // Отрисовка труб, если игра поставлена на паузу
                window.draw(pause); // Отрисовка паузы, если игра поставлена на паузу
            }
            window.display(); // Обновление экрана
        } 
        else { // Если игра окончена
            if (!lose_shown) { // Если не добавлена надпись 
                lose_shown = true; // Надпись добавлена
                window.draw(lose); // Отрисовка надписи
                window.display(); // Обновление экрана
            }
            else { // После добавление надписи
                if (Keyboard::isKeyPressed(Keyboard::R)) { // Если нажата кнопка "R"
                    return true; // Перезапустить игру
                } 
                if (Keyboard::isKeyPressed(Keyboard::Escape)) { // Если нажата кнопка "Esc"
                    return false; // Закрыть игру
                }
            }
        }
    }
}

// Основная функция main()
int main()
{
    // Переменная (игра продолжается, пока истинно)
    bool cont = true;

    while (cont) { // Пока истинно, игра будет перезапускаться
        cont = game(); // Если функция game() вернет значение false, программа завершит работу
    }
    // После закрытия игры, небольшой комментарий в консоли
    printf("Thank you for playing)"); 
}