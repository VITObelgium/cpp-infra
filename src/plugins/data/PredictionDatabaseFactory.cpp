#include "PredictionDatabaseFactory.h"

#include "Exceptions.h"
#include "PredictionDatabase.h"

#include "opaqconfig.h"

namespace opaq::factory {

using namespace infra;

std::unique_ptr<IPredictionDatabase> createPredictionDatabase(std::string_view type, std::string_view location, std::string_view user, std::string_view pass)
{
    if (type == "sqlite") {
        return std::make_unique<SqlitePredictionDatabase>(createSqliteConfig(location));
    } else if (type == "postgresql") {
#ifdef OPAQ_ENABLE_POSTGRESQL
        return std::make_unique<PostgresPredictionDatabase>(createPostgresConfig(location, user, pass));
#else
        (void)user;
        (void)pass;
        throw RuntimeError("Opaq was not compiled with postgresql support");
#endif
    }

    throw InvalidArgument("Invalid database type {}", type);
}

}
