    template <class K, class V, class D>
    class IterateKernel{
        // virtual void read_data(string& line, K& k, D& data, int &size) = 0;
        virtual void init_c(const K& k, V& delta, D& data) = 0;
        virtual const V& default_v() const = 0;
        virtual void init_v(const K& k, V& v, D& data) = 0;
        virtual void accumulate(V& a, const V& b) = 0;

        virtual void process_delta_v(const K& k, V& dalta, V& value, D& data) {
        }
        virtual void priority(V& pri, const V& value, const V& delta) = 0;
        virtual void g_func(const K& k, const V& delta, const V& value, const D& data, std::vector<std::pair<K, V> >* output) = 0;
    };