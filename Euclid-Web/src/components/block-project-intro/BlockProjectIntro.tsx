import {generateGradient} from "@/lib/utils";

const poweredByItems = [
    {
        title: "Integration of TRELLIS API",
        poweredBy: "Microsoft",
        link: "https://microsoft.github.io/TRELLIS/",
        image: "TRELLIS",
    },
    {
        title: "Hosted on Kubernetes Cluster",
        poweredBy: "Digital Ocean",
        link: "https://www.digitalocean.com/products/kubernetes",
        image: "Kubernetes",
    },
    {
        title: "Developed by Euclid Team",
        poweredBy: "Euclid",
        link: "https://github.com/Shellhacks-2025-Euclid/Euclid/",
        image: "Euclid",
    },
];

export default function BlockProjectIntro() {
    return (
        <section id={"project-intro"} className="flex gap-4 flex-col md:flex-row">
            <div className="w-full md:w-3/4 h-fit flex flex-col gap-4 md:sticky top-[106px] z-49">
                <div
                    className="bg-gray-200 dark:bg-[#1f1f1f] flex items-center justify-center rounded-md aspect-square md:aspect-[16/9]"
                    style={{backgroundImage: generateGradient()}}
                >
                    <p className="scroll-m-20 text-4xl md:text-6xl font-medium tracking-tight text-balance">
                        ShellHacks 2025 🦀
                    </p>
                </div>
                <h1 className="scroll-m-20 text-2xl md:text-5xl font-medium tracking-tight text-balance">
                    Euclid: From Axioms to Art
                </h1>
                <p className="text-muted-foreground text-sm mb-8 md:mb-0 md:text-base">
                    3D computer graphics software tool set powered with multi-agent AI systems.
                </p>
            </div>
            <div className="w-full md:w-1/4 flex flex-col gap-y-15 md:gap-y-20">
                {
                    poweredByItems.map((item, index) => (
                        <a
                            key={index}
                            className="w-full flex flex-col gap-4 hover:opacity-70 transition no-underline!"
                            href={item.link}
                            target="_blank"
                            rel="noreferrer"
                        >
                            <div
                                className="bg-gray-200 dark:bg-[#1f1f1f] flex items-center justify-center rounded-md aspect-square"
                                style={{backgroundImage: generateGradient()}}
                            >
                <span
                    className="scroll-m-20 text-3xl font-semibold tracking-tight first:mt-0 italic text-[#ffffff] dark:text-[#000000]">
                  {item.image}
                </span>
                            </div>
                            <p className="scroll-m-20 text-lg font-medium tracking-tight text-balance">
                                {item.title}
                            </p>
                            <p className="text-muted-foreground text-sm no-underline!">
                                Powered by <span className="underline">{item.poweredBy}</span>
                            </p>
                        </a>
                    ))
                }
            </div>
        </section>
    );
}