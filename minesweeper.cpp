﻿#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <string>

int main()
{
	srand(time(0));
	const int WINDOW_WIDTH = 352;
	const int WINDOW_HEIGHT = 400;
	sf::Vector2f screenSpace(WINDOW_WIDTH, WINDOW_HEIGHT);

	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "нень");
	bool inGame = true;
	std::string theme = "";

	const int fieldWidth = 10;
	const int fieldHeight = 10;

	struct Shake {
		int intensity = 25;
		int lifetime = 180;

		float offset_x = 0.f;
		float offset_y = 0.f;
	};

	struct Explosion {
		float x = 0.f;
		float y = 0.f;

		float scale_x = 0.f;
		float scale_y = 0.f;

		float motion_x = 0.f;
		float motion_y = 0.f;

		bool shouldShrink = false;
	};

	struct Flash {
		int density = 220;
		int lifetime = 25;
	};

	struct Dim {
		int density = 255;
		int lifetime = 2;

		bool shouldStart = true;
	};

	Explosion explosion;
	Shake shake;
	Flash flash;
	Dim dim;

	sf::Clock gameClock;
	bool gameOver = false;
	bool cellPressed = false;
	bool shouldRestart = true;

	bool menuScreen = true;
	bool winScreen = false;

	bool isWon = false;

	// CLOCKS ----------------------------------------------------------------------------
	sf::Clock flashClock;
	sf::Clock shakeClock;
	sf::Clock explosionClock;
	sf::Clock dimClock;

	// TEXTURES --------------------------------------------------------------------------
	sf::Texture menuBgTexture;
	sf::Texture buttonTexture;
	sf::Texture areaTexture;

	sf::Texture tilesTexture;

	sf::Texture explosionTexture;

	sf::Texture frameTexture;

	sf::Texture counterTexture;

	sf::Texture smileTexture;
	sf::Texture smilePressedTexture;
	sf::Texture smileHappyTexture;
	sf::Texture smileSadTexture;

	if (theme == "old") {
		menuBgTexture.loadFromFile("res/hud/mainMenuBg.png");
		buttonTexture.loadFromFile("res/hud/button.png");
		areaTexture.loadFromFile("res/hud/area.png");

		tilesTexture.loadFromFile("res/field/tiles_old.png");

		explosionTexture.loadFromFile("res/hud/explosion.png");

		frameTexture.loadFromFile("res/hud/frame_old.png");

		counterTexture.loadFromFile("res/hud/counter_old.png");

		smileTexture.loadFromFile("res/hud/smile_old.png");
		smilePressedTexture.loadFromFile("res/hud/smile_pressed_old.png");
		smileHappyTexture.loadFromFile("res/hud/smileHappy_old.png");
		smileSadTexture.loadFromFile("res/hud/smileSad_old.png");
	}
	else {
		menuBgTexture.loadFromFile("res/hud/mainMenuBg.png");
		buttonTexture.loadFromFile("res/hud/button.png");
		areaTexture.loadFromFile("res/hud/area.png");

		tilesTexture.loadFromFile("res/field/tiles.png");

		explosionTexture.loadFromFile("res/hud/explosion.png");

		frameTexture.loadFromFile("res/hud/frame.png");

		counterTexture.loadFromFile("res/hud/counter.png");

		smileTexture.loadFromFile("res/hud/smile.png");
		smilePressedTexture.loadFromFile("res/hud/smile_pressed.png");
		smileHappyTexture.loadFromFile("res/hud/smileHappy.png");
		smileSadTexture.loadFromFile("res/hud/smileSad.png");
	}

	// SPRITES ---------------------------------------------------------------------------
	sf::Sprite menu(menuBgTexture); //434343
	sf::Sprite area(areaTexture);

	sf::Sprite tiles(tilesTexture);
	sf::Sprite explosionObj(explosionTexture);
	sf::Sprite frame(frameTexture);

	sf::Sprite flagsCounterBg(counterTexture);
	sf::Sprite timeCounterBg(counterTexture);
	sf::Sprite smile(smileTexture);

	// SCREEN SPACE OBJECTS --------------------------------------------------------------
	sf::RectangleShape background(screenSpace);
	sf::RectangleShape flashObj(screenSpace);
	sf::RectangleShape dimObj(screenSpace);

	// FONTS -----------------------------------------------------------------------------
	sf::Font font;
	font.loadFromFile("res/fonts/Pixel Times.ttf");

	// COUNTERS --------------------------------------------------------------------------
	sf::Text flagsCounterT;
	flagsCounterT.setFont(font);
	flagsCounterT.setPosition(30, 14);

	sf::Text elapsedTimeT;
	elapsedTimeT.setFont(font);
	elapsedTimeT.setPosition(256, 14);
	elapsedTimeT.setString("0");

	flagsCounterBg.setPosition(26, 16);
	timeCounterBg.setPosition(252, 16);

	explosionObj.setOrigin(32, 32);
	area.setOrigin(128, 128); //ffffff

	smile.setOrigin(24, 24);
	smile.setPosition(WINDOW_WIDTH / 2, 32);

	if (theme == "old") {
		elapsedTimeT.setFillColor(sf::Color(216, 139, 139));
		flagsCounterT.setFillColor(sf::Color(216, 139, 139));

		background.setFillColor(sf::Color(106, 106, 106));
	}
	else {
		elapsedTimeT.setFillColor(sf::Color::White);
		flagsCounterT.setFillColor(sf::Color::White);

		background.setFillColor(sf::Color(64, 78, 99));
	}

	int field[fieldHeight + 2][fieldWidth + 2] = { 0 };
	int shownField[fieldHeight + 2][fieldWidth + 2] = { 0 };

	int gameOverField[fieldHeight + 2][fieldWidth + 2] = { 0 };

	int bombsAmount = 0;
	int bombsPercent = 80; //max <100 note: still bugged
	int flagsCounter = 0;

	int rightCounter = 0;
	int cellsOpened = 0;

	//
	//
	// UPDATE PART
	//
	//

	while (window.isOpen())
	{
		sf::Event event;

		sf::Vector2i pos = sf::Mouse::getPosition(window);
		int cell_x = (16 + pos.x) / 32;
		int cell_y = pos.y / 32 - 1;

		sf::Time gameTime = gameClock.getElapsedTime();

		if (shouldRestart) {

			bombsAmount = 0;
			
			//Empty or bomb
			for (int i = 1; i < fieldHeight + 1; i++)
			{
				for (int j = 1; j < fieldWidth + 1; j++)
				{
					shownField[i][j] = 10;

					if (rand() % (100 - bombsPercent) == 0) {
						field[i][j] = 9;
						bombsAmount += 1;
					}
					else {
						field[i][j] = 0;
					}
				}
			}
			flagsCounter = bombsAmount;

			//Number for mine detection
			for (int i = 1; i < fieldHeight + 1; i++)
			{
				for (int j = 1; j < fieldWidth + 1; j++)
				{
					if (field[i][j] != 9) {
						for (int k = -1; k < 2; k++)
						{
							for (int l = -1; l < 2; l++)
							{
								if (field[i + k][j + l] == 9) {
									field[i][j] += 1;
								}
							}
						}
					}
				}
			}
			
			//Shake
			shake.intensity = 25;
			shake.lifetime = 180;

			shake.offset_x = 0.f;
			shake.offset_y = 0.f;

			//Explosion
			explosion.x = 0.f;
			explosion.y = 0.f;

			explosion.scale_x = 0.f;
			explosion.scale_y = 0.f;

			explosion.motion_x = 0.f;
			explosion.motion_y = 0.f;

			explosion.shouldShrink = false;

			//Flash
			flash.density = 220;
			flash.lifetime = 25;

			//Dim
			dim.density = 255;
			dim.lifetime = 2;

			dim.shouldStart = true;

			gameOver = false;
			cellPressed = false;
			isWon = false;
			gameClock.restart();
			elapsedTimeT.setString("0");

			smile.setTexture(smileTexture);

			shouldRestart = false;
		}

		flagsCounterT.setString(std::to_string(flagsCounter));

		if (!gameOver && cellPressed) {
			elapsedTimeT.setString(std::to_string((int)gameTime.asSeconds()));
			//flagsCounterT.setString(std::to_string(window.getSize().x) + "; " + std::to_string(window.getSize().y));
			//flagsCounterT.setString(std::to_string(cellsOpened));
		}

		while (window.pollEvent(event))
		{
			if (event.key.code == sf::Mouse::Left) {
				if (event.type == sf::Event::MouseButtonReleased) {
					if (!gameOver) {
						if (shownField[cell_x][cell_y] == 10) {

							if (field[cell_x][cell_y] == 0) {
								for (int k = -1; k < 2; k++)
								{
									for (int l = -1; l < 2; l++)
									{
										if (shownField[cell_x + k][cell_y + l] == 10 && field[cell_x + k][cell_y + l] == 0) {
											shownField[cell_x + k][cell_y + l] = 0;
										}
										else {
											shownField[cell_x + k][cell_y + l] = field[cell_x + k][cell_y + l];
										}
									}
								}
							}
							else {
								shownField[cell_x][cell_y] = field[cell_x][cell_y];
							}

							if (field[cell_x][cell_y] == 9) {
								explosion.x = cell_x * 32 - 16;
								explosion.y = cell_y * 32 - 16;

								field[cell_x][cell_y] = 13;

								gameOver = true;
							}

							if (!cellPressed) {
								gameClock.restart();
								cellPressed = true;
							}
						}
					}

					if ((pos.x > 153 && pos.y > 8) && (pos.x < 201 && pos.y < 56)) {
						shouldRestart = true;
					}
					smile.setTexture(smileTexture);
				}
				if (event.type == sf::Event::MouseButtonPressed) {
					if ((pos.x > 153 && pos.y > 8) && (pos.x < 201 && pos.y < 56)) {
						smile.setTexture(smilePressedTexture);
					}
				}
			}
			if (event.key.code == sf::Mouse::Right) {
				if (event.type == sf::Event::MouseButtonPressed) {
					if (!gameOver) {
						if (shownField[cell_x][cell_y] == 10 && flagsCounter > 0) {
							flagsCounter -= 1;
							shownField[cell_x][cell_y] = 11;
							if (!cellPressed) {
								gameClock.restart();
								cellPressed = true;
							}
						}
						else if (shownField[cell_x][cell_y] == 11) {
							flagsCounter += 1;
							shownField[cell_x][cell_y] = 10;
						}
					}
				}
			}

			if (event.type == sf::Event::Closed)
				window.close();
		}

		int cellsOnField = fieldWidth * fieldHeight;
		cellsOpened = 0;
		rightCounter = 0;

		for (int i = 1; i < fieldHeight + 1; i++)
		{
			for (int j = 1; j < fieldWidth + 1; j++)
			{
				if (field[i][j] != 9 && shownField[i][j] == 11) {
					gameOverField[i][j] = 12;
				}
				else if (field[i][j] == 9 && shownField[i][j] == 11) {
					rightCounter += 1;
					gameOverField[i][j] = 11;
				} 
				else if (shownField[i][j] == 0 && cellPressed) {
					for (int k = -1; k < 2; k++)
					{
						for (int l = -1; l < 2; l++)
						{
							if (field[i + k][j + l] != 9) {
								shownField[i + k][j + l] = field[i + k][j + l];
							}
						}
					}
				}
				else {
					gameOverField[i][j] = field[i][j];
				}

				if (shownField[i][j] != 11 && shownField[i][j] != 10) {
					cellsOpened += 1;
				}
			}
		}

		if (cellsOnField - bombsAmount == cellsOpened && rightCounter == bombsAmount) {
			isWon = true;
		}

		if (isWon) {
			winScreen = true;
			smile.setTexture(smileHappyTexture);
		}

		sf::Time dimTime = dimClock.getElapsedTime();
		if (dimTime.asMilliseconds() >= dim.lifetime && dim.density > 0) {
			dim.density -= 1;
			dimClock.restart();
		}

		// ON GAME OVER -----------------------------------------------------------------------------------------

		if (gameOver) {
			sf::Time flashTime = flashClock.getElapsedTime();
			sf::Time shakeTime = shakeClock.getElapsedTime();
			sf::Time explosionTime = explosionClock.getElapsedTime();

			smile.setTexture(smileSadTexture);

			if (shakeTime.asMilliseconds() >= 25 && shake.intensity > 0) {
				shake.offset_x = rand() % shake.intensity - shake.intensity / 2;
				shake.offset_y = rand() % shake.intensity - shake.intensity / 2;

				if (shakeTime.asMilliseconds() >= shake.lifetime) {
					shake.intensity -= 1;
					shakeClock.restart();
				}
			}

			if (explosionTime.asMilliseconds() >= 25) {
				if (explosion.scale_x < 8 && !explosion.shouldShrink) {
					explosion.scale_x += 0.35;
					explosion.scale_y += 0.35;
				}
				else if (explosion.scale_x > 0) {
					explosion.scale_x -= 0.35;
					explosion.scale_y -= 0.35;
				}

				if (explosion.scale_x < 0) {
					explosion.scale_x = 0;
					explosion.scale_y = 0;
				}

				if (explosion.scale_x >= 8) {
					explosion.shouldShrink = true;
				}

				explosionClock.restart();
			}

			if (flashTime.asMilliseconds() >= flash.lifetime && flash.density > 0) {
				flash.density -= 1;
				flashClock.restart();
			}
		}

		// RENDER -----------------------------------------------------------------------------------------------
		window.clear();

		// INGAME -----------------------------------------------------------------------------------------------

		if (inGame) {
			window.draw(background);
			window.draw(frame);

			for (int i = 0; i < fieldWidth; i++)
			{
				for (int j = 0; j < fieldHeight; j++)
				{
					tiles.setPosition(i * 32 + shake.offset_x + 16, j * 32 + shake.offset_y + 64);
					if (!gameOver) {
						tiles.setTextureRect(sf::IntRect(32 * shownField[i + 1][j + 1], 0, 32, 32));
					}
					else {
						tiles.setTextureRect(sf::IntRect(32 * gameOverField[i + 1][j + 1], 0, 32, 32));
					}
					window.draw(tiles);

					//Logs mouse coordinates
					//std::cout << pos.x << "; " << pos.y << "\n";
				
					//Logs window size
					//std::cout << window.getSize().x << "; " << window.getSize().y << "\n";
				}	
			}

			window.draw(smile);

			// ON GAME OVER -----------------------------------------------------------------------------------------

			if (gameOver) {
				explosionObj.setPosition(explosion.x + 16, explosion.y + 64);
				explosionObj.setScale(explosion.scale_x, explosion.scale_y);

				flashObj.setPosition(0, 0);
				flashObj.setFillColor(sf::Color(255, 233, 173, flash.density));

				window.draw(explosionObj);
				window.draw(flashObj);
			}

			if (dim.shouldStart) {
				dimObj.setPosition(0, 0);
				dimObj.setFillColor(sf::Color(0, 0, 0, dim.density));
			}
			window.draw(dimObj);

			window.draw(flagsCounterBg);
			window.draw(timeCounterBg);
			window.draw(elapsedTimeT);
			window.draw(flagsCounterT);
		}
		else {
			if (winScreen) {

			}
			if (menuScreen) {
				area.setPosition(pos.x, pos.y);

				window.draw(area);
				window.draw(menu);
			}
		}


		window.display();
		// RENDER END ------------------------------------------------------------------------------------------
	}
	return 0;
}