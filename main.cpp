#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>
#include <tuple>
#include <fstream>

// Constants for random generation
const int MIN_ROWS = 100;
const int MAX_ROW_LENGTH = 1000;
const int MIN_ROW_LENGTH = 1;
const int MAX_PROFIT = 1000;
const int MIN_PROFIT = -1000;
const int MAX_DURATION = 1200;
const int MIN_DURATION = 1;

struct Trade {
    double profit;
    double duration;
};

using TradeRow = std::vector<Trade>;

// Function to generate a random trade row
TradeRow generate_random_row(std::mt19937 &rng) {
  std::uniform_int_distribution<int> length_dist(MIN_ROW_LENGTH, MAX_ROW_LENGTH);
  std::uniform_real_distribution<double> profit_dist(MIN_PROFIT, MAX_PROFIT);
  std::uniform_real_distribution<double> duration_dist(MIN_DURATION, MAX_DURATION);

  int length = length_dist(rng);
  TradeRow row(length);
  for (int i = 0; i < length; ++i) {
    row[i] = {profit_dist(rng), duration_dist(rng)};
  }
  return row;
}

// Function to calculate metrics for a trade row
std::tuple<double, double, double, double> calculate_metrics(const TradeRow &row) {
  double max_drawdown = 0;
  double max_drawdown_temp = 0;
  double max_drawdown_peak = 0;
  double total_duration = 0;
  double max_duration = 0;
  double cumulative_profit = 0;

  std::vector<double> cumulative_profits(row.size());
  for (size_t i = 0; i < row.size(); ++i) {
    cumulative_profit += row[i].profit;
    cumulative_profits[i] = cumulative_profit;
    total_duration += row[i].duration;
    if (row[i].duration > max_duration) {
      max_duration = row[i].duration;
    }
    if (cumulative_profit > max_drawdown_peak) {
      max_drawdown_peak = cumulative_profit;
    }
    double drawdown = max_drawdown_peak - cumulative_profit;
    if (drawdown > max_drawdown) {
      max_drawdown = drawdown;
    }
  }
  double average_duration = row.empty() ? 0 : total_duration / row.size();
  double recovery_factor = max_drawdown == 0 ? std::numeric_limits<double>::infinity() : cumulative_profit / max_drawdown;
  return {max_drawdown, average_duration, max_duration, recovery_factor};
}

int classify_strategy(double profit, double max_drawdown, double average_duration, double recovery_factor) {
  if (profit > 0 && recovery_factor > 1) {
    return 1;
  } else if (profit <= 0 && max_drawdown > -profit) {
    return -1;
  } else {
    return 0;
  }
}

int main(int argc, char* argv[]) {
  // Check for the correct number of arguments
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <output_file_path>" << std::endl;
    return 1;
  }

  std::string output_file_path = argv[1];

  std::random_device rd;
  std::mt19937 rng(rd());

  std::vector<TradeRow> trade_rows(MIN_ROWS);
  for (int i = 0; i < MIN_ROWS; ++i) {
    trade_rows[i] = generate_random_row(rng);
  }

  // Сохранение файла через аргумент cmd
  std::ofstream file(output_file_path);
  if (!file) {
    std::cerr << "Error: Could not open file " << output_file_path << " for writing." << std::endl;
    return 1;
  }

  for (const auto& row : trade_rows) {
    for (const auto& trade : row) {
      file << trade.profit << "," << trade.duration << ";";
    }
    file << "\n";
  }
  file.close();

  std::vector<int> classifications(MIN_ROWS);
  for (int i = 0; i < MIN_ROWS; ++i) {
    auto [max_drawdown, avg_duration, max_duration, recovery_factor] = calculate_metrics(trade_rows[i]);
    double total_profit = std::accumulate(trade_rows[i].begin(), trade_rows[i].end(), 0.0, [](double sum, const Trade &trade) {
        return sum + trade.profit;
    });
    classifications[i] = classify_strategy(total_profit, max_drawdown, avg_duration, recovery_factor);
  }

  for (int i = 0; i < MIN_ROWS; ++i) {
    std::cout << "Row " << i + 1 << ": Classification = " << classifications[i] << std::endl;
  }

  return 0;
}
