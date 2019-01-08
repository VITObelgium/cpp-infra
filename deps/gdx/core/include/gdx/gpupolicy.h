#pragma once

#include "internal/gpukernelfactory.h"

#include "gpucontext.h"
#include <boost/compute.hpp>
#include <type_traits>

namespace gdx::gpu
{

namespace bc = boost::compute;

template <typename T>
class Policy
{
public:
    static constexpr bool DirectMemoryAccess = false;

    using ge = bc::greater_equal<T>;
    using gt = bc::greater<T>;
    using le = bc::less_equal<T>;
    using lt = bc::less<T>;
    using ne = bc::not_equal_to<T>;

    using logical_and = bc::logical_and<T>;
    using logical_or = bc::logical_or<T>;
    using logical_not = bc::logical_not<T>;

    using value_type = T;
    using data_type = bc::vector<T>;

    using Factory = KernelFactory<T>;

    Policy()
    : _context(gpu::Context::instance())
    {
    }

    Policy(const Policy<T>&) = delete;
    Policy(Policy<T>&&)      = default;

    template<typename Iter1, typename Iter2>
    static void copy(Iter1 begin1, Iter1 end1, Iter2 begin2)
    {
        bc::copy(begin1, end1, begin2);
    }

    template<typename Iter>
    static void replace(Iter begin, Iter end, T oldValue, T newValue)
    {
        bc::replace(begin, end, oldValue, newValue);
    }

    template<typename Iter, typename UnaryPredicate>
    static bool all_of(Iter begin, Iter end, UnaryPredicate pred)
    {
        return bc::all_of(begin, end, pred);
    }

    template<typename Iter>
    static void fill(Iter begin, Iter end, T value)
    {
        bc::fill(begin, end, value);
    }

    template<typename Iter>
    static Iter min_element(Iter begin, Iter end, std::optional<double> nodata)
    {
        if constexpr(std::is_integral_v<value_type>)
        {
            if (nodata.has_value())
            {
                auto nodataInteger = static_cast<T>(*nodata);
                return boost::compute::min_element(begin, end, Factory::createIsLessForMinKernel(nodataInteger));
            }
            else
            {
                return boost::compute::min_element(begin, end);
            }
        }
        else
        {
            (void)nodata;
            return boost::compute::min_element(begin, end, Factory::createIsLessForMinKernel());
        }
    }

    template<typename Iter>
    static Iter max_element(Iter begin, Iter end, std::optional<double> nodata)
    {
        if constexpr(std::is_integral_v<value_type>)
        {
            if (nodata.has_value())
            {
                auto nodataInteger = static_cast<T>(*nodata);
                return boost::compute::max_element(begin, end, Factory::createIsLessForMaxKernel(nodataInteger));
            }
            else
            {
                return boost::compute::max_element(begin, end);
            }
        }
        else
        {
            (void)nodata;
            return boost::compute::max_element(begin, end, Factory::createIsLessForMaxKernel());
        }
    }

    template<typename Iter1, typename Iter2>
    static bool floatEqual(Iter1 begin1, Iter1 end1, Iter2 begin2, Iter2 end2, value_type tolerance)
    {
        if constexpr(std::is_integral_v<value_type>)
        {
            return boost::compute::equal(begin1, end1, begin2, end2);
        }
        else
        {
            BOOST_COMPUTE_CLOSURE(bool, float_equal, (boost::tuple<value_type, value_type> values), (tolerance),
            {
                float x = boost_tuple_get(values, 0);
                float y = boost_tuple_get(values, 1);

                if (isnan(x) && isnan(y))
                {
                    return true;
                }

                if (isnan(x) || isnan(y))
                {
                    return false;
                }

                return fabs(x - y) < tolerance;
            });

            return boost::compute::all_of(
                boost::compute::make_zip_iterator(boost::make_tuple(begin1, begin2)),
                boost::compute::make_zip_iterator(boost::make_tuple(end1, end2)),
                float_equal
            );
        }
    }

    template<typename Iter1, typename Iter2>
    static bool equal(Iter1 begin1, Iter1 end1, Iter2 begin2, Iter2 end2)
    {
        return bc::equal(begin1, end1, begin2, end2);
    }

    template<typename Iter1, typename OutputIter>
    static void equalTo(Iter1 begin, Iter1 end, OutputIter out, T value)
    {
        auto func = Factory::createEqualToKernel();
        bc::transform(begin, end, bc::make_constant_iterator(value), out, func);
    }

    template<typename Iter1, typename OutputIter>
    static void equalTo(Iter1 begin, Iter1 end, OutputIter out, T value, T noData)
    {
        auto func = Factory::createEqualToKernel(noData);
        bc::transform(begin, end, bc::make_constant_iterator(value), out, func);
    }

    template<typename Iter>
    static void greaterEqual(Iter begin, Iter end, value_type threshold)
    {
        auto func = Factory::createGreaterEqualKernel();
        bc::transform(begin, end, bc::make_constant_iterator(threshold), begin, func);
    }

    template<typename InputIter1, typename InputIter2, typename OutputIter>
    static void greaterEqualBinary(InputIter1 begin1, InputIter1 end1, InputIter2 begin2, T nodata, OutputIter output)
    {
        auto kernel = Factory::createGreaterEqualKernel(nodata);
        bc::transform(begin1, end1, begin2, output, kernel);
    }

    template<typename InputIter1, typename InputIter2, typename OutputIter>
    static void greaterEqualBinary(InputIter1 begin1, InputIter1 end1, InputIter2 begin2, OutputIter output)
    {
        auto kernel = Factory::createGreaterEqualKernel();
        bc::transform(begin1, end1, begin2, output, kernel);
    }

    template<typename Iter>
    static void lessEqual(Iter begin, Iter end, value_type threshold)
    {
        auto kernel = Factory::createLessEqualKernel();
        bc::transform(begin, end, bc::make_constant_iterator(threshold), begin, kernel);
    }

    template<typename InputIter1, typename InputIter2, typename OutputIter>
    static void lessEqualBinary(InputIter1 begin1, InputIter1 end1, InputIter2 begin2, T nodata, OutputIter output)
    {
        auto kernel = Factory::createLessEqualKernel(nodata);
        bc::transform(begin1, end1, begin2, output, kernel);
    }
    
    template<typename InputIter1, typename InputIter2, typename OutputIter>
    static void lessEqualBinary(InputIter1 begin1, InputIter1 end1, InputIter2 begin2, OutputIter output)
    {
        auto kernel = Factory::createLessEqualKernel();
        bc::transform(begin1, end1, begin2, output, kernel);
    }

    template<typename Iter>
    static void greater(Iter begin, Iter end, value_type threshold)
    {
        auto kernel = Factory::createGreaterKernel();
        bc::transform(begin, end, bc::make_constant_iterator(threshold), begin, kernel);
    }

    template<typename InputIter1, typename InputIter2, typename OutputIter>
    static void greaterBinary(InputIter1 begin1, InputIter1 end1, InputIter2 begin2, T nodata, OutputIter output)
    {
        auto kernel = Factory::createGreaterKernel(nodata);
        bc::transform(begin1, end1, begin2, output, kernel);
    }

    template<typename InputIter1, typename InputIter2, typename OutputIter>
    static void greaterBinary(InputIter1 begin1, InputIter1 end1, InputIter2 begin2, OutputIter output)
    {
        auto kernel = Factory::createGreaterKernel();
        bc::transform(begin1, end1, begin2, output, kernel);
    }

    template<typename Iter>
    static void less(Iter begin, Iter end, value_type threshold)
    {
        auto kernel = Factory::createLessKernel();
        bc::transform(begin, end, bc::make_constant_iterator(threshold, 0), begin, kernel);
    }

    template<typename InputIter1, typename InputIter2, typename OutputIter>
    static void lessBinary(InputIter1 begin1, InputIter1 end1, InputIter2 begin2, T nodata, OutputIter output)
    {
        auto kernel = Factory::createLessKernel(nodata);
        bc::transform(begin1, end1, begin2, output, kernel);
    }

    template<typename InputIter1, typename InputIter2, typename OutputIter>
    static void lessBinary(InputIter1 begin1, InputIter1 end1, InputIter2 begin2, OutputIter output)
    {
        auto kernel = Factory::createLessKernel();
        bc::transform(begin1, end1, begin2, output, kernel);
    }

    template<typename Iter>
    static void hasValue(Iter begin, Iter end)
    {
        bc::transform(begin, end, begin, Factory::createHasValueKernel());
    }

    template<typename Iter1, typename Iter2, typename ResultIter, typename BinaryOperation>
    static void transform(Iter1 begin1, Iter1 end1,
                          Iter2 begin2, ResultIter res,
                          BinaryOperation op)
    {
        bc::transform(begin1, end1, begin2, res, op);
    }

    template<typename Predicate, typename Iter, typename OutIter>
    static void performLogicalOperator(Iter begin1, Iter end1, Iter begin2, OutIter out)
    {
        auto kernel = Factory::createLogicalOperandKernel(Predicate());
        bc::transform(begin1, end1, begin2, out, kernel);
    }

private:
    bc::context& _context;
};

}
