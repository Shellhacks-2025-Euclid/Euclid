import { Button } from "@/components/ui/button";
import { generateGradient } from "@/lib/utils";

export default function BlockInstall() {
  return (
    <section id={"install"} className="flex gap-4 flex-col items-center">
      <div
        className="w-full bg-gray-200 dark:bg-[#1f1f1f] flex flex-col items-center justify-center gap-8 rounded-md py-[150px]"
        style={{ backgroundImage: generateGradient() }}
      >
        <p className="scroll-m-20 text-2xl md:text-5xl font-medium tracking-tight text-balance text-[#ffffff] dark:text-[#000000]">
          Get started with Euclid
        </p>

        <Button variant={"default"} size={"lg"} className="px-8">
          Download
        </Button>
      </div>
    </section>
  );
}