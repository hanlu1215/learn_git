#include <iostream>
#include <cmath>

void drawCircle(int radius) {
    const double aspect_ratio = 2.0; // Adjust for console character proportions
    const char fill_char = '*';     // Character to draw the circle
    const char empty_char = ' ';    // Character for empty space

    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius * aspect_ratio; x <= radius * aspect_ratio; ++x) {
            double distance = std::sqrt((x / aspect_ratio) * (x / aspect_ratio) + y * y);
            if (distance > radius - 0.5 && distance < radius + 0.5) {
                std::cout << fill_char;
            } else {
                std::cout << empty_char;
            }
        }
        std::cout << std::endl;
    }
}

int main() {
    int radius;
    std::cout << "Enter the radius of the circle: ";
    std::cin >> radius;

    if (radius > 0) {
        drawCircle(radius);
    } else {
        std::cout << "Radius must be a positive integer!" << std::endl;
    }
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.ignore();
    std::cin.get();
    return 0;
}