namespace sick
{
    class Policy
    {
    public:
        enum class Level
        {
            Strict,
            Flexible
        };

        Level level;
        bool allow;

        Policy(Level l = Level::Strict, bool a = false)
            : level(l)
            , allow(a)
        {
        }
    };

    class SecureKernel
    {
    private:
        std::mutex mtx;
        std::vector<std::string> logs;

    public:
        SecureKernel() = default;

        void Log(const std::string& msg)
        {
            std::lock_guard<std::mutex> lock(mtx);
            logs.push_back("[" + msg + "]");
        }

        bool Run(std::function<void()> task, const Policy& p)
        {
            Log("Core Loop Init");

            if (!p.allow && p.level == Policy::Level::Strict)
            {
                Log("Strict Shield Active");
            }

            try
            {
                task();
                Log("Success");
                return true;
            }
            catch (...)
            {
                Log("Violation Detected");
                return false;
            }
        }

        void Dump() const
        {
            for (const auto& l : logs)
            {
                std::cout << l << "\n";
            }
        }
    };

    class SecureInterface
    {
    private:
        std::shared_ptr<SecureKernel> krn;
        Policy plc;

    public:
        SecureInterface(std::shared_ptr<SecureKernel> k, Policy p)
            : krn(k)
            , plc(p)
        {
        }

        bool Kick(std::function<void()> payload)
        {
            if (!krn)
            {
                return false;
            }

            return krn->Run(payload, plc);
        }
    };
}

int main()
{
    auto core = std::make_shared<sick::SecureKernel>();
    sick::Policy rigid(sick::Policy::Level::Strict, false);
    sick::SecureInterface iface(core, rigid);

    auto safe = []() 
    {
        std::cout << "[SICK] Core logic operational.\n";
    };

    iface.Kick(safe);

    std::cout << "\n=== SICK Diagnostics ===\n";
    core->Dump();

    return 0;
}
