#include <torch/extension.h>
#include <iostream>

using namespace torch::indexing;
using namespace std;


torch::Tensor isotonic_l2(torch::Tensor y) {
    auto n = y.size(0);
    auto target = torch::arange(n);
    auto c = torch::ones_like(y);
    auto sums = torch::ones_like(y);
    auto sol = torch::zeros_like(y);
    
    for (int i = 0; i < n; i++) {
        sol[i] = y[i];
        sums[i] = y[i];
    }

    int i = 0;
    while (i < n) {
        auto k = target[i].item<int>();
        if (k == n) {
            break;
        }
        if (sol[i].item<double>() > sol[k].item<double>()) {
            i = k;
            continue;
        }
        auto sum_y = sums[i];
        auto sum_c = c[i];
        while (true) {
            // Non-singleton increasing subsequence is finished,
            // update first entry.
            auto prev_y = sol[k].item<double>();
            sum_y += sums[k];
            sum_c += c[k];
            k = target[k].item<int>() + 1;
            if ((k == n) || (prev_y > sol[k].item<double>())) {
                sol[i] = sum_y / sum_c;
                sums[i] = sum_y;
                c[i] = sum_c;
                target[i] = k;
                target[k - 1] = i;
                if (i > 0) {
                    // Backtrack if we can.  This makes the algorithm
                    // single-pass and ensures O(n) complexity.
                    i = target[i - 1].item<int>();
                }
                // Otherwise, restart from the same point
                break;
            }
        }
    }
    // Reconstruct the solution
    i = 0;
    while (i < n) {
        auto k = target[i].item<int64_t>() + 1;
        sol.index_put_({Slice(i + 1, k, None)}, sol[i]);
        i = k;
    }
    return sol;
}
    

PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
  m.def("isotonic_l2", &isotonic_l2, "Isotonic L2");
}