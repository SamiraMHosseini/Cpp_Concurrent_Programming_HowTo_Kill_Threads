#include <conio.h>
struct SharedResource
{
	SharedResource() :
		cv(), mtx(), killflag(false)
	{

	}
	//These are resources that are shared between threads

	std::condition_variable cv;
	std::mutex mtx;
	bool killflag;
};
class A : public BannerBase
{

public:
	A() = delete;
	A(const A&) = default;
	A& operator=(const A&) = default;
	~A() = default;
	A(const char* const name) :
		BannerBase(name), counter(0)
	{

	}
	void operator ()(SharedResource& sharedResource) //Functor 
	{
		START_BANNER;
		while (true)
		{

			std::unique_lock<std::mutex> lock(sharedResource.mtx);
			//Wait for 1400ms unless there is an interrupt or a signal
			if (sharedResource.cv.wait_for(lock, 1400ms, [&]() -> bool { return sharedResource.killflag; }))
			{
				break;
			}

			Debug::out("%d \n", this->counter);
			++(this->counter);


		}

	}

private:
	int counter;

};
class B : public BannerBase
{
public:
	B() = delete;
	B& operator=(const B&) = default;
	B(const B&) = default;
	~B() = default;
	B(const char* const name) :
		BannerBase(name), counter(0x10000)
	{

	}
	void operator()(SharedResource& sharedResource)
	{
		START_BANNER;

		while (true)
		{

			std::unique_lock<std::mutex> lock(sharedResource.mtx);
			//Wait for 1200ms unless there is an interrupt or a signal
			bool flag = sharedResource.cv.wait_for(lock, 1200ms, [&]() -> bool {return sharedResource.killflag; });
			if (flag)
			{
				break;
			}
			Debug::out("0x%x \n", this->counter);
			this->counter = this->counter - 1;

		}
	}

private:
	unsigned int counter;


};

class C : public BannerBase
{

public:

	C() = delete;
	C(const C&) = default;
	C& operator=(const C&) = default;
	~C() = default;
	C(const char* const name) :
		BannerBase(name), strings{ "apple", "orange" ,"banana", "lemon" }
	{

	}

	void operator() (SharedResource& sharedResource)
	{

		START_BANNER;
		int index{ 0 };
		while (true)
		{

			std::unique_lock<std::mutex> lock(sharedResource.mtx);
			//Wait for 1000ms unless there is an interrupt or a signal
			if (sharedResource.cv.wait_for(lock, 1000ms, [&]()->bool { return sharedResource.killflag; }))
			{
				break;
			}
			Debug::out("%s \n", this->strings[index]);
			++index;
			if (index > 3)
			{
				index = 0;
			}

		}
	}

private:


	const char* const strings[4];
};

class D : public BannerBase
{
public:
	D() = delete;
	D(const D&) = default;
	D& operator=(const D&) = default;
	~D() = default;

	D(const char* const name) :
		BannerBase(name)
	{

		
		
	}
	void operator() (SharedResource& sharedReference)
	{
		START_BANNER;
		const char* const sourceStr = "<0><1><2><3><4><5><6><7><8>";
		char* destStr = new char[strlen(sourceStr) + 1];
		memcpy(destStr, sourceStr, strlen(sourceStr) + 1);
		size_t len{ strlen(sourceStr) };
		int index{ 0 };
		while (true)
		{

			std::unique_lock<std::mutex> lock(sharedReference.mtx);
			bool flag = sharedReference.cv.wait_for(lock, 1600ms, [&]()->bool {return sharedReference.killflag; });
			if (flag)
			{
				break;
			}

			
			Debug::out("%s \n", destStr);
			if (strlen(destStr) <= MINIMUM_LEN)
			{
				index = 3;
				memcpy(destStr, sourceStr, len);
			}
			*(destStr + len + --index) = 0;
			*(destStr + len + --index) = 0;
			*(destStr + len + --index) = 0;
		}

		delete destStr;
	}
private:

	
	const size_t MINIMUM_LEN = 3;
	
};

int main()
{

	START_BANNER_MAIN("Main");

	A a("A");
	B b("B");
	C c("C");
	D d("D");

	//Resources shared between threads 
	SharedResource sharedResource;

	//Spawn threads
	std::thread thA(a, std::ref(sharedResource));
	std::thread thB(b, std::ref(sharedResource));
	std::thread thC(c, std::ref(sharedResource));
	std::thread thD(d, std::ref(sharedResource));

	//Detach the threads
	thA.detach();
	thB.detach();
	thC.detach();
	thD.detach();

	//Key Press
	_getch();

	//Signal to the threads
	{

		Debug::out("Key is pressed\n");
		std::lock_guard<std::mutex> lock(sharedResource.mtx);
		sharedResource.killflag = true;
	}
	sharedResource.cv.notify_all();

	std::this_thread::sleep_for(1s); //This is going to get fixed in future versions. 

}