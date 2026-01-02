#include <iosfwd>
#include <chrono>
#include <vector>
#include <random>
#include <cstdint>
#include <cstddef>
#include <iostream>

// include the two lists we made
#include "ll_list_pool.hpp"
#include "ll_intrusive_list.hpp"

/*
Lets configure some values.
 */

static constexpr std::size_t N_SMALL = 10;
static constexpr std::size_t N_LARGE = 1000000; // 1 million
static constexpr std::size_t OPS = 5000000; // 5 million

/*
 Timing helpers (in nanoseconds calculation)
 */

template <class F>
uint64_t time_ns(F&& f)
{
 auto start = std::chrono::steady_clock::now();
 f();
 auto end = std::chrono::steady_clock::now();
 return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

// Payloads

struct order
{
 uint64_t id;
};
struct intrusive_order
{
 intrusive_hook hook;
 uint64_t id;
};

// small demonstration of the lists we made

void demo_small()
{
 std::cout << "\n=== Small example: 10 elements ===\n";
 {
  std::cout << "\n[Pool-backed list]\n";
  ll_list_pool<order> lst(16);
  for (uint64_t i = 0; i < N_SMALL; i++)
   lst.emplace_back(order{i});

  std::cout << "Initial order: ";
  for (auto & [id] : lst)
  {
   std::cout << id << " ";
  }

  std::cout << "\n";
  std::cout << "Splicing last ->front \n";
  auto last = --lst.end();
  lst.splice(lst.begin(),last);
  std::cout << "After splice: ";
  for (auto & [id] : lst)
  {
   std::cout << id << " ";
  }

  std::cout << "\n";

 }

 {
  std::cout << "\n[Intrusive list]\n";

  intrusive_list lst;
  std::vector<intrusive_order> orders;
  orders.reserve(N_SMALL);
  for (uint64_t i = 0; i < N_SMALL; ++i)
  {
   orders.push_back({{},i});
   lst.push_back(&orders.back().hook);
  }
  std::cout << "Initial order: ";
  for (auto* h = lst.front(); h!= lst.end(); h = h->next)
  {
   const auto* o = reinterpret_cast<intrusive_order*>(
    reinterpret_cast<char*>(h) - offsetof(intrusive_order, hook));
   std::cout << o->id << " ";
  }
  std::cout << "\n";
  std::cout << "splicing last -> front\n";
  if (!lst.empty()) lst.splice(lst.front(),lst.back());

  std::cout << "After splice: ";
  for (auto* h = lst.front(); h!= lst.end(); h = h->next)
  {
   auto* o = reinterpret_cast<intrusive_order*>(
    reinterpret_cast<char*>(h) - offsetof(intrusive_order,hook));
   std::cout << o->id <<" ";
  }
  std::cout << "\n";
 }
}

/*
 * BENCHMARK: FULL TRAVERSAL (POINTER CHASING)
 */

void benchmark_iteration()
{
 std::cout << "\n=== Benchmark: full traversal ===\n";
 ll_list_pool<order> pool_list(N_LARGE);
 for (uint64_t i = 0; i < N_LARGE; i++)
 {
  pool_list.emplace_back(order{i});
 }
 intrusive_list intr_list;
 std::vector<intrusive_order> intr_orders(N_LARGE);
 for (uint64_t i = 0; i < N_LARGE; i++)
 {
  intr_orders[i].id = i;
  intr_list.push_back(&intr_orders[i].hook);
 }

 uint64_t t_pool = time_ns([&]
 {
  uint64_t sum = 0;
  for (auto it = pool_list.begin(); it!= pool_list.end(); ++it)
  {
   sum+= (*it).id;
  }
  (void)sum;
 });

 uint64_t t_intr = time_ns([&]
 {
  uint64_t sum = 0;
  for (auto* h = intr_list.front(); h != intr_list.end(); h = h->next)
  {
   auto* o = reinterpret_cast<intrusive_order*>(
    reinterpret_cast<char*>(h) - offsetof(intrusive_order,hook));
   sum+= o->id;
  }
  (void)sum;
 });

 std::cout << "Pool list traversal (ns): " << t_pool << "\n";
 std::cout << "Intrusive list traversal (ns): " << t_intr << "\n";
}

/*
 * BENCHMARK SPLICE HOT PATH (PROMOTION)
 */

void benchmark_splice()
{
 std::cout << "\n=== Benchmark: repeated splice ===\n";
 ll_list_pool<order> pool_list(N_LARGE);
 std::vector<ll_list_pool<order>::iterator> pool_iters;

 for (uint64_t i =0; i < N_LARGE; i++)
 {
  pool_iters.push_back(pool_list.emplace_back(order{i}));
 }

 intrusive_list intr_list;
 std::vector<intrusive_order> intr_orders(N_LARGE);
 for (uint64_t i = 0; i < N_LARGE; i++)
 {
  intr_orders[i].id = i;
  intr_list.push_back(&intr_orders[i].hook);
 }

 std::mt19937 rng(42);
 std::uniform_int_distribution<std::size_t> pick(0,N_LARGE-1);

 uint64_t t_pool = time_ns([&]
 {
  for (std::size_t i = 0; i < OPS; ++i)
  {
   pool_list.splice(pool_list.begin(),pool_iters[pick(rng)]);
  }
 });

 uint64_t t_intr = time_ns([&]
 {
  for (std::size_t i = 0; i < OPS; ++i)
  {
   intr_list.splice(intr_list.front(), &intr_orders[pick(rng)].hook);
  }
 });

 std::cout << "Pool list splice (ns):   " << t_pool << "\n";
 std::cout << "Intrusive list splice (ns): " << t_intr << "\n";
}

int main()
{
 demo_small();
 benchmark_iteration();
 benchmark_splice();
}
















