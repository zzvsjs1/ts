#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include <random>
#include <limits>
#include <numeric>

namespace dir_vec
{
	// 4 directions.
	const olc::vd2d UP = {  0, -1 };
	const olc::vd2d DOWN = {  0, 1 };
	const olc::vd2d LEFT = {  -1, 0 };
	const olc::vd2d RIGHT = { 1, 0 };
}

enum class DIR_ENUM { TOP_LEFT, TOP_RIGHT, LOWER_LEFT, LOWER_RIGHT };

[[nodiscard]]
static olc::vd2d getFourPointVec(const olc::vd2d& topLeft, const olc::vd2d& size, const DIR_ENUM dir) noexcept
{
	switch (dir)
	{
		case DIR_ENUM::TOP_LEFT: return topLeft;
		case DIR_ENUM::TOP_RIGHT: return { topLeft.x + size.x, topLeft.y };
		case DIR_ENUM::LOWER_LEFT: return { topLeft.x, topLeft.y + size.y };
		case DIR_ENUM::LOWER_RIGHT: return { topLeft.x + size.x, topLeft.y + size.y };
		default: return topLeft;
	}
}

[[nodiscard, maybe_unused]]
static bool isRectOverlap(
	const olc::vd2d& leftTopLeft,
	const olc::vd2d& leftSize,
	const olc::vd2d& rightTopLeft,
	const olc::vd2d& rightSize
) noexcept
{
	const auto leftLowerRight = getFourPointVec(leftTopLeft, leftSize, DIR_ENUM::LOWER_RIGHT);
	const auto rightLowerRight = getFourPointVec(rightTopLeft, rightSize, DIR_ENUM::LOWER_RIGHT);

	return leftTopLeft.x < rightLowerRight.x
		&& leftLowerRight.x > rightTopLeft.x
		&& leftTopLeft.y < rightLowerRight.y
		&& leftLowerRight.y > rightTopLeft.y;
}

[[nodiscard]]
static bool isPixelInRect(
	const olc::vd2d& rectTopLeft,
	const olc::vd2d& rectWAndH,
	const olc::vd2d& pixelPos
) noexcept
{
	const auto rectLowerRight = rectTopLeft + rectWAndH;
	return pixelPos.x >= rectTopLeft.x && pixelPos.y >= rectTopLeft.y && pixelPos.x <= rectLowerRight.x && pixelPos.y <= rectLowerRight.y;
}


// APPLE

constexpr auto APPLE_RADIUS = 6;

struct Apple
{
	olc::vi2d mPos;

	void draw(olc::PixelGameEngine& engine) const
	{
		// Hollow circle
		//engine.FillCircle(mPos, APPLE_RADIUS, olc::RED);
		engine.FillCircle(mPos, APPLE_RADIUS, olc::RED);
	}
};

// SNAKE

constexpr auto DEFAULT_SNAKE_LEN = 3;
constexpr auto SNAKE_SIZE = 5;
constexpr auto SNAKE_SPEED = 40;


namespace 
{
	const olc::vd2d SNAKE_SIZE_VEC = { SNAKE_SIZE, SNAKE_SIZE };
	const olc::vd2d DOUBLE_EPSILON = { std::numeric_limits<double>::epsilon(), std::numeric_limits<double>::epsilon() };
}

struct Snake
{
	struct SnakeBody
	{
		olc::vd2d pos;
	};

	Snake(const int32_t width, const int32_t height)
		: mBody(), mDirVec2d(dir_vec::LEFT)
	{
		// Add body.
		for (size_t i = 0; i < DEFAULT_SNAKE_LEN; ++i)
		{
			mBody.emplace_back(SnakeBody{ { static_cast<double>(width) / 2 + i,  static_cast<double>(height ) / 2 } });
		}

		//// Add head.
		//mBody.emplace_back(olc::vd2d{ width / 2, height / 2 });

		//// Add body.
		//for (size_t i = 1; i < DEFAULT_SNAKE_LEN; ++i)
		//{
		//	const auto& [pos] = mBody.back();
		//	mBody.emplace_back(SnakeBody{ { pos.x + SNAKE_SIZE + 1, pos.y } });
		//}
	}

	void normalUpdate(const float fElapsedTime) noexcept
	{
		for (size_t i = mBody.size() - 1; i > 0; --i)
		{
			mBody[i].pos = mBody[i - 1].pos;
		}

		getSnakeHead().pos += mDirVec2d * SNAKE_SPEED * fElapsedTime;

		//for (size_t i = mBody.size() - 1; i > 0; --i)
		//{
		//	mBody[i].pos = mBody[i - 1].pos;
		//}

		//const auto newPos = mDirVec2d * SNAKE_SIZE;
		//getSnakeHead().pos += newPos;
	}

	void specialUpdate() noexcept
	{
		for (size_t i = mBody.size() - 1; i > 0; --i)
		{
			mBody[i].pos = mBody[i - 1].pos;
		}

		const auto newPos = mDirVec2d * SNAKE_SIZE;
		getSnakeHead().pos += newPos;
	}

	void draw(olc::PixelGameEngine& engine) const
	{
		for (const auto& [pos] : mBody)
		{
			//engine.DrawRect(pos, SNAKE_SIZE_VEC);
			engine.Draw(pos);
		}
	}

	void toUp() noexcept
	{
		mDirVec2d = dir_vec::UP;
	}

	void toRight() noexcept
	{
		mDirVec2d = dir_vec::RIGHT;
	}

	void toDown() noexcept
	{
		mDirVec2d = dir_vec::DOWN;
	}

	void toLeft() noexcept
	{
		mDirVec2d = dir_vec::LEFT;
	}

	[[nodiscard]]
	SnakeBody& getSnakeHead() noexcept
	{
		return mBody.front();
	}

	[[nodiscard]]
	const SnakeBody& getSnakeHead() const noexcept
	{
		return mBody.front();
	}

	[[nodiscard]]
	bool isDeath(const int32_t width, const int32_t height) const noexcept
	{
		const auto& headPos = getSnakeHead().pos;

		//if (inRectRange(headPos, SNAKE_SIZE_VEC, width, height))
		if (headPos.x < 0 || headPos.x > width || headPos.y < 0 || headPos.y > height)
		{
			return true;
		}

		for (size_t i = 1, size = mBody.size(); i < size; ++i)
		{
			//if (isRectOverlap(
			//	headPos, SNAKE_SIZE_VEC, mBody[i].pos, SNAKE_SIZE_VEC
			//))
			if (static_cast<olc::vi2d>(headPos) == static_cast<olc::vi2d>(mBody[i].pos))
			{
				return true;
			}
		}

		return false;
	}

	void extendBody(const float fElapsedTime)
	{
		const auto backUp = mBody.back();
		normalUpdate(fElapsedTime);
		mBody.emplace_back(backUp);
	}

	::std::vector<SnakeBody> mBody;
	olc::vd2d mDirVec2d;
};

// GAME

#define NUM_OF_APPLES (3)

static const olc::vd2d RECT_LEN_VEC = { APPLE_RADIUS * 2, APPLE_RADIUS * 2 };

class Game final : public olc::PixelGameEngine
{
public:

	Game()
		: olc::PixelGameEngine(),
		mSnake(ScreenWidth(), 
		ScreenHeight()),
	mApples(),
	mScore(),
	isOver(),
	mRandEngine(std::random_device{}()),
	mRandX(),
	mRandY()

	{
		sAppName = "Snake";
		mRandX = ::std::uniform_int_distribution(APPLE_RADIUS, ScreenWidth() - APPLE_RADIUS);
		mRandY = ::std::uniform_int_distribution(APPLE_RADIUS, ScreenHeight() - APPLE_RADIUS);
	}

	void resetGame()
	{
		addApples();
		mSnake = Snake(ScreenWidth(), ScreenHeight());
		isOver = false;
	}

	void addApples()
	{
		while (mApples.size() < NUM_OF_APPLES)
		{
			mApples.emplace_back(Apple{ { mRandX(mRandEngine), mRandY(mRandEngine)}});
		}
		//mApples.emplace_back(Apple{ { 50, 50 } });
		//mApples.emplace_back(Apple{ { 100, 50 } });
		//mApples.emplace_back(Apple{ { 150, 80 } });
	}

	bool OnUserCreate() override
	{
		resetGame();
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);

		if (isOver)
		{
			handleKeyInput(fElapsedTime);
			DrawString(20, ScreenHeight() / 3, std::string("Game over! You got " + std::to_string(mScore) + " point!"));
			return true;
		}

		handleKeyInput(fElapsedTime);

		isEatApple(fElapsedTime);
		addApples();


		// Draw all entities.
		mSnake.draw(*this);
		for (const auto& apple : mApples)
		{
			apple.draw(*this);
		}

		// Check if we need to stop the game.
		isOver = mSnake.isDeath(ScreenWidth(), ScreenHeight());

		return true;
	}

	void isEatApple(const float fElapsedTime)
	{
		for (size_t i = 0, size = mApples.size(); i < size; ++i)
		{
			for (const auto& [pos] : mSnake.mBody)
			{
				const olc::vd2d cirToRectPos
					= { static_cast<double>(mApples[i].mPos.x - APPLE_RADIUS),
					static_cast<double>(mApples[i].mPos.y - APPLE_RADIUS)
					};

				if (isPixelInRect(cirToRectPos, RECT_LEN_VEC, pos))
				{
					mSnake.extendBody(fElapsedTime);
					mSnake.extendBody(fElapsedTime);
					mSnake.extendBody(fElapsedTime);
					mSnake.extendBody(fElapsedTime);
					mApples.erase(
							mApples.cbegin() + static_cast<::std::vector<Apple>::difference_type>(i)
					);
					++mScore;
					return;
				}
			}
			//if (mApples[i].mPos == mSnake.getSnakeHead().pos)
			//{
			//	mSnake.extendBody();
			//	mApples.erase(
			//		mApples.cbegin() + static_cast<::std::vector<Apple>::difference_type>(i)
			//	);
			//	++mScore;
			//	return;
			//}
		}
	}

	void handleKeyInput(const float fElapsedTime)
	{
		// Make sure the user not go to opposite direction.
		if (GetKey(olc::Key::UP).bHeld && mSnake.mDirVec2d != dir_vec::DOWN)
		{
			mSnake.toUp();
		}
		else if (GetKey(olc::Key::RIGHT).bHeld && mSnake.mDirVec2d != dir_vec::LEFT)
		{
			mSnake.toRight();

		}
		else if (GetKey(olc::Key::DOWN).bHeld && mSnake.mDirVec2d != dir_vec::UP)
		{
			mSnake.toDown();
		}
		else if (GetKey(olc::Key::LEFT).bHeld && mSnake.mDirVec2d != dir_vec::RIGHT)
		{
			mSnake.toLeft();
		}
		else if (GetKey(olc::Key::W).bHeld)
		{
			resetGame();
		}

		mSnake.normalUpdate(fElapsedTime);
	}

	Snake mSnake;
	std::vector<Apple> mApples;
	uint64_t mScore;
	bool isOver;
	std::default_random_engine mRandEngine;
	std::uniform_int_distribution<int32_t> mRandX;
	std::uniform_int_distribution<int32_t> mRandY;

};

#ifndef _DEBUG
int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#else
int main()
#endif
{
	if (Game game; game.Construct(256, 256, 4, 4, false, true))
	{
		game.Start();
	}

	return 0;
}
