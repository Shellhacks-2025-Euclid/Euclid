export default function BlockWhatDoesItLookLike() {
  return (
    <section id={"what-does-it-look-like"} className="flex gap-4 flex-col bg-gray-200 dark:bg-[#1f1f1f] rounded-md p-4">
      <div className="w-full flex items-center flex-col md:flex-row gap-8">
        <div
          className="w-full aspect-[6/4] md:w-1/2 flex justify-center items-center rounded-md bg-[#1f1f1f] dark:bg-gray-200"
          style={{
            backgroundImage: "url(/examples/screen.png)",
            backgroundSize: "100%",
            backgroundPosition: "center",
            backgroundRepeat: "no-repeat",
          }}
        >
        </div>
        <div className="w-full md:w-1/2 flex flex-col gap-4">
          <h2 className="scroll-m-20 text-2xl md:text-3xl font-medium tracking-tight text-balance">
            What does it look like?
          </h2>
          <p className="text-muted-foreground text-sm md:text-base">
            The Euclid interface is designed to be user-friendly and intuitive, allowing users to easily navigate
            through the various features and functionalities. The layout is clean and organized, with clear labels
            and icons to guide users through their tasks.
          </p>
        </div>
      </div>
    </section>
  );
}