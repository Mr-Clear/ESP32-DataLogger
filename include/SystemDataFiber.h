#pragma once

#include <Fiber.h>
#include <Observable.h>

#include <memory>

class SHT3x;

class SystemDataFiber : public Fiber {
public:
  struct Data {
    uint32_t heapSize;
    uint32_t freeHeap;
    
    bool operator==(const Data &o) const;
    bool operator!=(const Data &o) const;
  };

  SystemDataFiber();
  ~SystemDataFiber();

  const Observable<Data> &data();
  const Observable<uint32_t> &heapSize();
  const Observable<uint32_t> &freeHeap();
  
protected:
  void setup() override;
  void loop() override;

private:
  Observable<Data> _data;
  Observable<uint32_t> _heapSize;
  Observable<uint32_t> _freeHeap;
};
