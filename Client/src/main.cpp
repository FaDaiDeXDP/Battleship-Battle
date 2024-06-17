#include <SFML/Graphics.hpp>

int main()
{
    // 创建一个窗口
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Window");

    // 主循环
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // 清除窗口
        window.clear();

        // 在这里绘制图形

        // 显示窗口内容
        window.display();
    }

    return 0;
}