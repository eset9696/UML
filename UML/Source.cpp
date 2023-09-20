#include <iostream>
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
	double consumption;
	double consumption_per_second;
	bool is_started;
public:
	double get_consumption() const
	{
		return consumption;
	}
	double get_consumption_per_second() const
	{
		return consumption_per_second;
	}

	void set_consumption_per_second(double consumption)
	{
		consumption_per_second = consumption * 3e-5;
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

	Engine(double consumption) : DEFAULT_CONSUMPTION
		(
			consumption < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
			consumption > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
			consumption
		)
	{
		this->consumption = DEFAULT_CONSUMPTION;
		set_consumption_per_second(consumption);
		is_started = false;
		cout << "Engine is ready:\t" << this << endl;
	}
	~Engine()
	{
		cout << "Engine is over:\t" << this << endl;
	}

	void info() const
	{
		cout << "Consuption:\t" << consumption << " liters per 100 km.\n";
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
	bool driver_inside;
	bool is_move;

	struct Threads
	{
		std::thread panel_thread;
		std::thread engine_idle_thread;
		std::thread engine_drive_thread;
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

	Car(double consumption, int volume, int max_speed) : engine(consumption), tank(volume), MAX_SPEED
		(
			max_speed < MAX_SPEED_LOWER_LEVEL ? MAX_SPEED_LOWER_LEVEL :
			max_speed > MAX_SPEED_UPPER_LEVEL ? MAX_SPEED_UPPER_LEVEL :
			max_speed
		)
	{
		this->speed = 0;
		driver_inside = false;
		is_move = false;
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
				if (driver_inside)get_out();
				else get_in();
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
			case 'D':case'd':
				if (engine.started()) gas();
				break;
			case 'S':case's':
				if (engine.started()) to_brake();
				break;
			case Escape:
				get_out();
			}
			if (tank.get_fuel_level() == 0)
			{
				stop();
				to_brake();
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

	void gas()
	{
		if(engine.started())
		{
			is_move = true;
			if(!threads.engine_drive_thread.joinable()) threads.engine_drive_thread = std::thread(&Car::engine_drive, this);
			//if(speed == 0) threads.engine_drive_thread = std::thread(&Car::engine_drive, this);
			if (speed < MAX_SPEED) change_speed(10);
		}
	}

	void to_brake()
	{
		if (get_speed() > 0) change_speed(-10);
		if(get_speed() == 0)
		{
			is_move = false;
			if (threads.engine_drive_thread.joinable())
			{
				threads.engine_drive_thread.join();
			}
		}
	}

	void stop()
	{
		engine.stop();
		if (threads.engine_idle_thread.joinable()) threads.engine_idle_thread.join();
	}

	void engine_idle()
	{
		while ( engine.started() && tank.get_fuel_level())
		{
			if (!is_move)
			{
				tank.give_fuel(engine.get_consumption_per_second());
				to_brake();
			}
			std::this_thread::sleep_for(1s);
		}
		
	}

	void engine_drive()
	{
		while (is_move && tank.give_fuel((engine.get_consumption() / 10))) // каждую секунду сгорает 10% топлива от расхода на 100 км
		{
			change_speed(-1); //пока не жмем на газ машина катится и следовательно тормозит под действием сил трения
			if (get_speed() == 0) is_move = false;
			std::this_thread::sleep_for(1s);
		}
	}

	void panel() const
	{
		while (driver_inside)
		{
			system("CLS");
			cout << "Speed:\t" << this->get_speed() << " km/h." << endl;
			cout << "Fuel level:\t" << tank.get_fuel_level() << " liters.\n";
			cout << "Engine is:\t" << (engine.started() ? " started." : " stopped.") << endl;
			if(tank.get_fuel_level() < 5) cout << "LOW FUEL" << endl;
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