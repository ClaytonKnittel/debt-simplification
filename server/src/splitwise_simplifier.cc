#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <optional>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"

#include "proto/debts.pb.h"
#include "server/src/csv/csv.h"
#include "server/src/expense_simplifier/debt_graph.h"

ABSL_FLAG(std::optional<std::string>, input_csv, std::nullopt,
          "The splitwise-exported CSV file of expenses.");

absl::StatusOr<debt_simpl::DebtList> BuildDebtListFromSplitwiseExpenseReport(
    const std::string& report_path) {
  rapidcsv::Document document(report_path);
  std::cout << "Columns: " << document.GetColumnCount() << std::endl;
  for (size_t i = 0; i < document.GetColumnCount(); i++) {
    std::cout << document.GetColumnName(i) << std::endl;
  }

  const size_t category_column = document.GetColumnIdx("Category");

  debt_simpl::DebtList debts;
  const size_t num_rows = document.GetRowCount();
  for (size_t i = 0; i < num_rows; i++) {
    const std::vector<std::string> row = document.GetRow<std::string>(i);
    if (row.size() != document.GetColumnCount() ||
        document.GetCell<std::string>(category_column, i) == "Payment" ||
        document.GetCell<std::string>(category_column, i) == "") {
      // Ignore payment columns.
      continue;
    }

    std::vector<debt_simpl::Cents> balances;
    // The first 5 columns are metadata, and all the following are the debts for
    // each person.
    std::transform(
        row.begin() + 5, row.end(), std::back_inserter(balances),
        [](const std::string& cell) {
          size_t endptr;
          double res = std::stod(cell, &endptr);
          if (endptr != cell.size()) {
            // TODO: Throw error.
          }
          return static_cast<debt_simpl::Cents>(std::round(res * 100));
        });

    size_t payer_idx = SIZE_MAX;
    for (size_t j = 0; j < balances.size(); j++) {
      if (balances[j] > 0) {
        payer_idx = j;
        break;
      }
    }

    if (payer_idx == SIZE_MAX) {
      std::cerr << "Expected payer in row " << i << std::endl;
      return absl::InternalError(absl::StrCat("Expected payer in row ", i));
    }

    for (size_t j = 0; j < balances.size(); j++) {
      std::cout << balances[j] << ", ";
      if (j != payer_idx) {
        if (balances[j] > 0) {
          return absl::InternalError(
              absl::StrCat("Unexpected multiple payers in row ", i));
        }
        debt_simpl::Transaction transaction;
        transaction.set_lender(document.GetColumnName(payer_idx + 5));
        transaction.set_receiver(document.GetColumnName(j + 5));
        transaction.set_cents(-balances[j]);
        debts.add_transactions();
      }
    }
    std::cout << std::endl;
  }

  return debts;
}

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);

  auto input_csv = absl::GetFlag(FLAGS_input_csv);
  if (!input_csv.has_value()) {
    std::cerr << "Must provide flag `--input_csv`" << std::endl;
    return -1;
  }

  const absl::StatusOr<debt_simpl::DebtList> debts =
      BuildDebtListFromSplitwiseExpenseReport(input_csv.value());
  return 0;
}
