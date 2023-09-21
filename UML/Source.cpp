#include <iostream>
#include <windows.h>
#include <conio.h>
#include <thread>

using std::cin;
using std::cout;
using std::endl;

using namespace std::chrono_literals;

#define Enter 13
#define Escape 27

#define MIN_TANK_VOLUME  20
#define MAX_TANK_VOLUME  120

class Tank
{
	const int VOLUME;
	double fuel_level;

public:
	int get_VOLUME() const
	{
		return VOLUME;
	}

	double get_fuel_level() const
	{
		return fuel_level;
	}

	void fill(double fuel)
	{
		if (fuel < 0) return;
		if (fuel_level + fuel < VOLUME) fuel_level += fuel;
		else fuel_level = VOLUME;
	}
	

	Tank(int volume) : VOLUME
	(
		volume < MIN_TANK_VOLUME ? MIN_TANK_VOLUME :
		volume > MAX_TANK_VOLUME ? MAX_TANK_VOLUME :
		volume
	)
	{
		this->fuel_level = 0;
		cout << "Tank is ready:\t" << this << endl;
	}

	~Tank()
	{
		cout << "Tank is over:\t" << this << endl;
	}

	void info()const
	{
		cout << "Volume:\t" << get_VOLUME() << " liters.\n";
		cout << "Fuel level:\t" << get_fuel_level() << " liters." << endl;
	}

	double give_fuel(double amount)
	{
		fuel_level -= amount;
		if (fuel_level < 0) fuel_level = 0;
		return fuel_level;
	}
};

#define MIN_ENGINE_CONSUMPTION 3
#define MAX_ENGINE_CONSUMPTION 30

class Engine
{
	const double DEFAULT_CONSUMPTION;
	const double DEFAULT_CONSUMPTION_PER_SECOND;
	double consumption_per_second;
	bool is_started;
public:
	double get_DEFAULT_CONSUMPTION_PER_SECOND() const
	{
		return DEFAULT_CONSUMPTION_PER_SECOND;
	}
	double get_consumption_per_second() const
	{
		return consumption_per_second;
	}

	void set_consumption_per_second(int speed)
	{
		if(speed == 0) consumption_per_second = DEFAULT_CONSUMPTION_PER_SECOND;
		else if(speed < 60) consumption_per_second = DEFAULT_CONSUMPTION_PER_SECOND * 20 / 3;
		else if(speed < 100) consumption_per_second = DEFAULT_CONSUMPTION_PER_SECOND * 14 / 3;
		else if(speed < 140) consumption_per_second = DEFAULT_CONSUMPTION_PER_SECOND * 20 / 3;
		else if(speed < 200) consumption_per_second = DEFAULT_CONSUMPTION_PER_SECOND * 25 / 3;
		else if(speed < 250) consumption_per_second = DEFAULT_CONSUMPTION_PER_SECOND * 10;
	}

	void start()
	{
		is_started = true;
	}

	void stop()
	{
		is_started = false;
	}

	bool started() const
	{
		return is_started;
	}

	Engine(double default_consumption_per_second) : DEFAULT_CONSUMPTION
		(
			default_consumption_per_second < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
			default_consumption_per_second > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
			default_consumption_per_second
		), DEFAULT_CONSUMPTION_PER_SECOND(DEFAULT_CONSUMPTION * 3e-5)
	{
		set_consumption_per_second(0);
		is_started = false;
		cout << "Engine is ready:\t" << this << endl;
	}
	~Engine()
	{
		cout << "Engine is over:\t" << this << endl;
	}

	void info() const
	{
		cout << "Consuption:\t" << DEFAULT_CONSUMPTION << " liters per 100 km.\n";
		cout << "Consuption per second:\t" << consumption_per_second << " liters per second.\n";
	}
};


#define MAX_SPEED_LOWER_LEVEL 30
#define MAX_SPEED_UPPER_LEVEL 400
class Car
{
	Engine engine;
	Tank tank;
	int speed;
	const int MAX_SPEED;
	int accelleration;
	bool driver_inside;

	struct Threads
	{
		std::thread panel_thread;
		std::thread engine_idle_thread;
		std::thread free_wheeling_thread;
	}threads;

public:
	int get_speed() const
	{
		return speed;
	}

	int get_MAX_SPEED() const
	{
		return MAX_SPEED;
	}

	void change_speed(int acceleration)
	{
		this->speed += acceleration;
		if (speed < 0) speed = 0;
		if (speed > MAX_SPEED)this->speed = MAX_SPEED;
	}

	Car(double defoult_consumption_per_second, int volume, int max_speed, int accelleration = 10) : engine(defoult_consumption_per_second), tank(volume), MAX_SPEED
		(
			max_speed < MAX_SPEED_LOWER_LEVEL ? MAX_SPEED_LOWER_LEVEL :
			max_speed > MAX_SPEED_UPPER_LEVEL ? MAX_SPEED_UPPER_LEVEL :
			max_speed
		)
	{
		this->speed = 0;
		driver_inside = false;
		this->accelleration = accelleration;
		cout << "Car is ready to go:\t" << this << endl;
	}
	~Car()
	{
		cout << "Car is over:\t" << this << endl;
	}

	void get_in()
	{
		driver_inside = true;
		threads.panel_thread = std::thread(&Car::panel, this);
	}

	void get_out()
	{
		driver_inside = false;
		if (threads.panel_thread.joinable()) threads.panel_thread.join();
		system("CLS");
		cout << "Outside" << endl;
	}

	void control()
	{
		char key;
		do
		{
			key = 0;
			if (_kbhit()) // Функция _kbhit() 
			{
				key = _getch();
			}
			//key = _getch();
			switch (key)
			{
			case Enter:
				if (driver_inside && speed == 0) get_out();
				else if (!driver_inside && speed == 0) get_in();
				break;
			case 'F': case 'f':
			{
				if(driver_inside)
				{
					cout << "Для начала нужно выйти из машины" << endl;
					break;
				}
				double fuel;
				cout << "Введите объем топлива:\t"; cin >> fuel;
				tank.fill(fuel);
			}
			case 'I':case 'i':
				if (engine.started()) stop();
				else start();
				break;
			case 'W':case 'w': accellerate(); break;
			case 'S':case 's': slow_down(); break;
			case Escape:
				speed = 0;
				stop();
				get_out();
			}
			if (speed <= 0) engine.set_consumption_per_second(speed = 0);
			if (speed == 0 && threads.free_wheeling_thread.joinable()) threads.free_wheeling_thread.join();
			if (tank.get_fuel_level() == 0)
			{
				stop();
			}
		} while (key != Escape);
	}

	void start()
	{
		if(driver_inside && tank.get_fuel_level())
		{
			engine.start();
			threads.engine_idle_thread = std::thread(&Car::engine_idle, this);
		}
	}

	void stop()
	{
		engine.stop();
		if (threads.engine_idle_thread.joinable()) threads.engine_idle_thread.join();
	}

	void engine_idle()
	{
		while ( engine.started() && tank.give_fuel(engine.get_consumption_per_second()))
			std::this_thread::sleep_for(1s);
		
	}

	void free_wheeling()
	{
		while(--speed > 0)
		{
			std::this_thread::sleep_for(1s);
			engine.set_consumption_per_second(speed);
		}
	}

	void accellerate()
	{
		if (engine.started() && driver_inside)
		{
			speed += accelleration;
			if (speed > MAX_SPEED) speed = MAX_SPEED;
			if (!threads.free_wheeling_thread.joinable()) threads.free_wheeling_thread = std::thread(&Car::free_wheeling, this);
			std::this_thread::sleep_for(1s);
		}
	}

	void slow_down()
	{
		if (driver_inside)
		{
			speed -= accelleration;
			if (speed < 0) speed = 0;
			std::this_thread::sleep_for(1s);
		}
	}

	void panel() const
	{
		while (driver_inside)
		{
			system("CLS");
			for (int i = 0; i < speed / 3; i++)
			{
				cout << "|";
			}
			cout << endl;
			cout << "Fuel level:\t" << tank.get_fuel_level() << " liters.\t";
			if (tank.get_fuel_level() < 5)
			{
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, 0xCE);
				cout << "LOW FUEL" << endl;
				SetConsoleTextAttribute(hConsole, 0x07);
			}
			cout << endl;
			cout << "Engine is:\t" << (engine.started() ? " started." : " stopped.") << endl;
			cout << "Speed:\t" << this->get_speed() << " km/h." << endl;
			cout << "Consumption per second:\t" << engine.get_consumption_per_second() << " liters." << endl;
			std::this_thread::sleep_for(100ms);
		}
	}
};

#define TANK_CHECK
void main()
{
	setlocale(LC_ALL, "");

	Car BMW(10, 40, 250);
	BMW.control();
}