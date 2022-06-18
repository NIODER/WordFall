#include <iostream>
#include <Windows.h>
#include <random>
#include <thread>
#include <mutex>
#include <conio.h>

std::mutex mtx;

struct Letter
{
    int position;
    char letter;

    Letter(int pos, char let)
    {
        position = pos;
        letter = let;
    }
};

struct Map
{
private:
    const char ru_symbols[47]{
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
        'й', 'ц', 'у', 'к', 'е', 'н', 'г', 'ш', 'щ', 'з', 'х', 'ъ',
        'ф', 'ы', 'в', 'а', 'п', 'р', 'о', 'л', 'д', 'ж', 'э', '\\',
        'я', 'ч', 'с', 'м', 'и', 'т', 'ь', 'б', 'ю', '.', 'ё'
    };
    const char en_symbols[46] {
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']',
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/'
    };
    char* symbols;
    char** map;
    int height, width, letters_count, delta, max_letters, lowest, score, symbolsLen;
    Letter** letters;
    char background;
    bool endgame;

public:
    Map(int height, int width, char background, int delta, bool is_rus)
    {
        this->height = height;
        this->width = width;

        this->delta = delta;
        this->max_letters = this->height / this->delta;
        this->letters_count = 0;
        letters = new Letter*[max_letters];
        this->lowest = 0;

        map = new char* [this->height];
        for (int i = 0; i < this->height; i++)
            map[i] = new char[this->width];
        for (int i = 0; i < this->height; i++)
        {
            for (int j = 0; j < this->width; j++)
                map[i][j] = background;
        }
        this->background = background;
        this->endgame = false;
        this->score = 0;
        if (is_rus)
        {
            symbolsLen = 47;
            symbols = new char[symbolsLen];
            for (int i = 0; i < symbolsLen; i++)
                symbols[i] = ru_symbols[i];
        }
        else
        {
            symbolsLen = 46;
            symbols = new char[symbolsLen];
            for (int i = 0; i < symbolsLen; i++)
                symbols[i] = en_symbols[i];
        }
    }

    ~Map()
    {
        for (int i = 0; i < height; i++)
            delete[] map[i];
        delete[] map;
        for (int i = 0; i < max_letters; i++)
            delete letters[i];
        delete letters;
        delete[] symbols;
        delete[] en_symbols;
        delete[] ru_symbols;
    }

    void print()
    {
        mtx.lock();
        system("cls");
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
                std::cout << map[i][j];
            std::cout << std::endl;
        }
        for (int i = 0; i < width; i++)
        {
            std::cout << "#";
        }
        std::cout << std::endl;
        std::cout << "Score: " << score << std::endl;
        mtx.unlock();
    }

    bool isGameOver()
    {
        return endgame && (height < lowest);
    }

    void spawn()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        letters[letters_count] = new Letter(gen() % width, symbols[gen() % symbolsLen]);
        letters_count++;
    }

    void animation()
    {
        if (letters_count == 0)
            spawn();
        int y = lowest;
        for (int i = 0; i < letters_count && y >= 0 && y < height; i++)
        {
            map[y][letters[i]->position] = letters[i]->letter;
            if (y - delta >= 0)
                map[y - delta][letters[i]->position] = background;
            y -= delta;
        }
        if (y >= height)
        {
            mtx.lock();
            endgame = true;
            mtx.unlock();
        }
        print();
        lowest += delta;
    }

    void cycle()
    {
        animation();
        spawn();
    }

    Letter& firstLetter()
    {
        return **letters;
    }

    void delete_first()
    {
        mtx.lock();
        lowest -= delta;
        map[lowest][firstLetter().position] = ' ';
        letters_count--;
        for (int i = 0; i < letters_count; i++)
            letters[i] = letters[i + 1];
        mtx.unlock();
        print();
    }

    void sc()
    {
        this->score += 100;
    }
};

void getCh(Map* map)
{
    while (!map->isGameOver())
    {
        if (map->firstLetter().letter == (char)_getch())
        {
            map->delete_first();
            map->sc();
        }
    }
}

void start(int height, int width, int millies, bool is_ru)
{
    Map* map = new Map(height, width, ' ', 1, is_ru);

    map->cycle();

    std::thread th(getCh, map);

    while (!map->isGameOver())
    {
        map->cycle();
        Sleep(millies);
    }
    th.join();
    system("cls");
    std::cout << "Game Over" << std::endl;
}

int main()
{
    setlocale(LC_ALL, "Russian");
    int lvl, millies, lang;
    char answer;
    std::cout << "Предпочитаемая сложность" << std::endl;
    std::cout << "1 - легкая" << std::endl;
    std::cout << "2 - сложная" << std::endl;
    std::cout << "3 - очень сложная" << std::endl;
    std::cout << "4 - невозможно" << std::endl;

difficulty:
    std::cin >> lvl;
    switch (lvl)
    {
    case 1:
        millies = 2000;
        break;
    case 2:
        millies = 1000;
        break;
    case 3:
        millies = 500;
        break;
    case 4:
        millies = 100;
        break;
    default:
        std::cout << "Введите сложность" << std::endl;
        goto difficulty; // да, это гото...
        break;
    }
    std::cout << "Раскладка" << std::endl;
    std::cout << "Русская йцукен - 1" << std::endl;
    std::cout << "Английская qwerty - 2" << std::endl;
    std::cin >> lang;
    do
    {
        start(20, 20, millies, lang == 1);
        std::cout << "Play again? y/n" << std::endl;
        answer = _getch();
    } while (answer == 'y');

    return 0;
}
