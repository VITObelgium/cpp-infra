// generated by .\ddl2cpp .\PredictionScheme.ddl .\PredictionScheme OPAQ
#ifndef OPAQ_PREDICTIONSCHEME_H
#define OPAQ_PREDICTIONSCHEME_H

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace OPAQ
{
  namespace Predictions_
  {
    struct Basetime
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "Basetime";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T Basetime;
            T& operator()() { return Basetime; }
            const T& operator()() const { return Basetime; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct Date
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "Date";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T Date;
            T& operator()() { return Date; }
            const T& operator()() const { return Date; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
    struct Value
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "Value";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T Value;
            T& operator()() { return Value; }
            const T& operator()() const { return Value; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::floating_point, sqlpp::tag::can_be_null>;
    };
    struct Model
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "Model";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T Model;
            T& operator()() { return Model; }
            const T& operator()() const { return Model; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct Pollutant
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "Pollutant";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T Pollutant;
            T& operator()() { return Pollutant; }
            const T& operator()() const { return Pollutant; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct Aggregation
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "Aggregation";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T Aggregation;
            T& operator()() { return Aggregation; }
            const T& operator()() const { return Aggregation; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct Station
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "Station";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T Station;
            T& operator()() { return Station; }
            const T& operator()() const { return Station; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::can_be_null>;
    };
    struct ForecastHorizon
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "ForecastHorizon";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T ForecastHorizon;
            T& operator()() { return ForecastHorizon; }
            const T& operator()() const { return ForecastHorizon; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::can_be_null>;
    };
  }

  struct Predictions: sqlpp::table_t<Predictions,
               Predictions_::Basetime,
               Predictions_::Date,
               Predictions_::Value,
               Predictions_::Model,
               Predictions_::Pollutant,
               Predictions_::Aggregation,
               Predictions_::Station,
               Predictions_::ForecastHorizon>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "predictions";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T predictions;
        T& operator()() { return predictions; }
        const T& operator()() const { return predictions; }
      };
    };
  };
}
#endif