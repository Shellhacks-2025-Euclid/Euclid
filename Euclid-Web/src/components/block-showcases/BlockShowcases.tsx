"use client";

import React from "react";
import { type CarouselApi } from "@/components/ui/carousel";
import { Card, CardContent } from "@/components/ui/card";
import { Carousel, CarouselContent, CarouselItem } from "@/components/ui/carousel";
import { Button } from "@/components/ui/button";
import { ArrowLeftIcon, ArrowRightIcon } from "lucide-react";

export default function BlockShowcases() {
  const [api, setApi] = React.useState<CarouselApi>();
  return (
    <section id={"showcases"} className="flex gap-4 flex-col py-[150px]">
      <div className="w-full flex flex-col gap-2 mb-8">
        <h2 className="scroll-m-20 text-2xl md:text-3xl font-medium tracking-tight text-balance">
          What you can create with Euclid
        </h2>
        <p className="text-muted-foreground text-sm md:text-base">
          {"However, Euclid's true potential lies in your imagination."}
        </p>
      </div>

      <Carousel
        opts={{
          align: "start",
          dragFree: true,
          containScroll: "trimSnaps",
          loop: true,
        }}
        className="w-full"
        setApi={setApi}
      >
        <CarouselContent>
          {Array.from({ length: 5 }).map((_, index) => (
            <CarouselItem key={index} className="md:basis-1/2 lg:basis-1/3">
              <div>
                <Card>
                  <CardContent className="flex aspect-square items-center justify-center p-6">
                    <span className="text-3xl font-semibold">{index + 1}</span>
                  </CardContent>
                </Card>
              </div>
            </CarouselItem>
          ))}
        </CarouselContent>
      </Carousel>

      <div className="w-full">
        <div className="w-full flex justify-center gap-2">
          <Button
            variant={"secondary"}
            className="w-1/2 md:w-[120px] cursor-pointer flex justify-between"
            onClick={() => api?.scrollPrev()}
          >
            <ArrowLeftIcon />
            Previous
          </Button>
          <Button
            variant={"secondary"}
            className="w-1/2 md:w-[120px] cursor-pointer flex justify-between"
            onClick={() => api?.scrollNext()}
          >
            Next
            <ArrowRightIcon />
          </Button>
        </div>
      </div>
    </section>
  );
}