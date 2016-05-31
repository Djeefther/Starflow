#ifndef GENETIC_ALGORITHM_
#define GENETIC_ALGORITHM_

#include <vector>

//template<class FuncInitializePopulation, class RandomGenerator>

template<class FuncInitializePopulation, class FuncEvaluate,
class FuncIsOptimal, class FuncCrossOver, class FuncMutate,
class FuncShowPopulation = nullptr_t,
class RandomGenerator = std::default_random_engine>

	auto genetic_algorithm(size_t population_size, size_t keep_population_next_gen,
		size_t num_generation, FuncInitializePopulation initialize_population,
		FuncEvaluate evaluate, FuncIsOptimal is_optimal, FuncCrossOver crossover,
		double mutate_probability, FuncMutate mutate, FuncShowPopulation show_population = nullptr,
		RandomGenerator rgenerator = std::default_random_engine(time(NULL)))

	-> std::vector<decltype(initialize_population(rgenerator))> //return final population

{
	using Tind = decltype(initialize_population(rgenerator)); //Type Individual
	using Teva = decltype(evaluate(Tind())); //Type evaluation

	std::vector<Tind> population;
	std::vector<Teva> evaluation_population;
	std::vector<Teva> accumulated_evaluation_population;

	std::vector<Tind*> selected_population;//to be parents
	std::vector<Tind> next_population;

	//initial population
	population.reserve(population_size);
	for (size_t i = 0; i < population_size; i++) {
		population.push_back(initialize_population(rgenerator));
	}

	evaluation_population.resize(population_size);
	accumulated_evaluation_population.resize(population_size);
	selected_population.resize(population_size);
	next_population.resize(population_size);

	for (size_t generation = 0; generation < num_generation; generation++) {
		std::sort(population.begin(), population.end(),
			[=](const auto& a, const auto& b) {return evaluate(a) > evaluate(b); });
		//Optional "kind of a DEBUG"

		//showing population
		if (!std::is_same<decltype(show_population), nullptr_t>::value) {
			show_population(const_cast<const std::vector<Tind>&>(population));
		}

		//Evaluating and maybe finishing
		Teva total_evaluation = 0;
		bool optimal_solution = false;
		for (size_t i = 0; i < population_size; i++) {
			evaluation_population[i] = evaluate(population[i]);
			total_evaluation += evaluation_population[i];
			accumulated_evaluation_population[i] = total_evaluation;

			if (is_optimal(evaluation_population[i])) {
				optimal_solution = true;
			}
		}

		if (optimal_solution) {
			break;
		}

		//Choose parents population parents and the first keep_population_next_gen goes to the next generation
		size_t next_population_count = 0;

		std::uniform_int_distribution<Teva> roulette(0, total_evaluation - 1);
		for (size_t i = 0; i < population_size; i++) {
			Teva value_roulette = roulette(rgenerator);

			auto it_acc = std::lower_bound(accumulated_evaluation_population.begin(),
				accumulated_evaluation_population.end(),
				value_roulette);

			auto id = it_acc - accumulated_evaluation_population.begin();

			selected_population[i] = &population[id];

			if (next_population_count < keep_population_next_gen) {
				next_population[next_population_count] = population[id];
				next_population_count++;
			}
		}

		//Crossover
		std::uniform_int_distribution<size_t> random_population(0, population_size - 1);
		for (size_t i = next_population_count; i + 1 < population_size; i += 2) {
			auto parent1 = random_population(rgenerator);
			auto parent2 = random_population(rgenerator);

			crossover(rgenerator,
				*selected_population[parent1], *selected_population[parent2],
				next_population[i], next_population[i + 1]);

			next_population_count = i + 1;//last atribuate element
		}

		//if odd, do just the last 
		if (next_population_count < population_size) {
			auto parent1 = random_population(rgenerator);
			auto parent2 = random_population(rgenerator);

			Tind trash_individual;
			crossover(rgenerator,
				*selected_population[parent1], *selected_population[parent2],
				next_population.back(), trash_individual);
		}

		//mutate
		std::uniform_real_distribution<double> random_mutate(0, 1);
		for (auto& i : next_population) {
			if (random_mutate(rgenerator) < mutate_probability) {
				mutate(rgenerator, i);
			}
		}

		//assing next iteration
		population.swap(next_population);
	}

	//Before returning ordered

	return population;
}


#endif