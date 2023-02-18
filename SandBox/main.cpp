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
		BannerBase(name), counter(0), name(name)
	{

	}
	void operator ()(SharedResource& sharedResource) //Functor 
	{
		START_BANNER;
		Debug::SetCurrentName(name);
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
	const char* const name;
};
class B : public BannerBase
{
public:
	B() = delete;
	B& operator=(const B&) = default;
	B(const B&) = default;
	~B() = default;
	B(const char* const name) :
		BannerBase(name), counter(0x10000), name(name)
	{

	}
	void operator()(SharedResource& sharedResource)
	{
		START_BANNER;
		Debug::SetCurrentName(name);
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
	const char* const name;

};

class C : public BannerBase
{

public:

	C() = delete;
	C(const C&) = default;
	C& operator=(const C&) = default;
	~C() = default;
	C(const char* const name) :
		BannerBase(name), name(name), strings{ "apple", "orange" ,"banana", "lemon" }
	{

	}

	void operator() (SharedResource& sharedResource)
	{

		START_BANNER;
		int index{ 0 };
		Debug::SetCurrentName(name);
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

	const char* const name;
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
		BannerBase(name), name(name), source("<0><1><2><3><4><5><6><7><8>")
	{
		
		this->dest = new char[strlen(this->source)+1];
		memcpy(this->dest, this->source, strlen(this->source)+1);
	}
	void operator() (SharedResource& sharedReference)
	{
		START_BANNER;
		Debug::SetCurrentName(name);
		size_t len{ strlen(this->source) };
		int index{ 0 };
		while (true)
		{

			std::unique_lock<std::mutex> lock(sharedReference.mtx);
			bool flag = sharedReference.cv.wait_for(lock, 1600ms, [&]()->bool {return sharedReference.killflag; });
			if (flag)
			{
				break;
			}
			
			*(this->dest + len+index ) = '\0'; //OR *(this->dest + len + index) = 0;
			Debug::out("%s \n", this->dest);
			index -= MINIMUM_LEN;
			if (strlen(this->dest)<= MINIMUM_LEN)
			{
				index = 0;
				memcpy(this->dest, this->source, len);
			}
		}
		
		delete dest;
	}
private:
	const char* const name;
	const char* const source;
	const int MINIMUM_LEN = 3;
	char* dest;
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